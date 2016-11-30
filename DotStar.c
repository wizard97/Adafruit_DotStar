#include "DotStar.h"
#include "MemManager.h"
#include "FunctionLib.h"
#include "board.h"

// 4 start bytes plus 0xFF for full brighteness + end crap
#define BUFFER_BYTES(NUMLEDS) (START_BYTES + 4*NUMLEDS + END_BYTES(NUMLEDS))
//#define PACKING_BYTES(NUMLEDS) (4 + NUMLEDS + END_BYTES(NUMLEDS))
#define END_BYTES(NUMLEDS) ((NUMLEDS + 15) / 16)
#define START_BYTES 4

static void DotStar_spiEnd(DotStar_t *self);
static void DotStar_spiInit(DotStar_t *self);
static void DotStar_spiTransferCallback(SPI_Type *base, dspi_master_handle_t *handle,
        status_t status, void *userData);

void DotStar_spiTransferCallback(SPI_Type *base, dspi_master_handle_t *handle,
        status_t status, void *userData)
{
    DotStar_t *self = (DotStar_t*)userData;
    self->isFinished = TRUE;
}


DotStar_t DotStar_create(uint16_t num, uint8_t o, SPI_Type *spi)
{
    DotStar_t ret;
    ret.numLEDs = num;
    ret.brightness = 0;
    ret.pixels = NULL;
    ret.rOffset = o & 3;
    ret.gOffset = (o >> 2) & 3;
    ret.bOffset = (o >> 4) & 3;
    ret.isFinished = TRUE;

    DotStar_updateLength(&ret, num);
    // get default config
    DSPI_MasterGetDefaultConfig(&ret.masterConfig);
    //store spi type
    ret.spi = spi;
    return ret;
}


void DotStar_deinit(DotStar_t *self)
{
    if(self->xfer.txData)
      MEM_BufferFree(self->xfer.txData);

     DotStar_spiEnd(self);
}

void DotStar_begin(DotStar_t *self)
{
    DotStar_spiInit(self);
}


// Length can be changed post-constructor for similar reasons (sketch
// config not hardcoded).  But DON'T use this for "recycling" strip RAM...
// all that reallocation is likely to fragment and eventually fail.
// Instead, set length once to longest strip.
void DotStar_updateLength(DotStar_t *self, uint16_t n)
{
    if(self->xfer.txData) MEM_BufferFree(self->xfer.txData);

    uint16_t bytes = (self->rOffset == self->gOffset) ?
      n + ((n + 3) / 4) : // MONO: 10 bits/pixel, round up to next byte
      n * 3;              // COLOR: 3 bytes/pixel

    self->xfer.txData = (uint8_t *)MEM_BufferAlloc(BUFFER_BYTES(n) + bytes);
    self->xfer.rxData = NULL;
	self->xfer.dataSize = BUFFER_BYTES(n);

    if(self->xfer.txData) {
      // 4 start bytes
      for(uint8_t i=0; i<4; i++) self->xfer.txData[i] = 0x00;
      // end bytes
      for(uint16_t i=BUFFER_BYTES(n); i<BUFFER_BYTES(n) - END_BYTES(n); i++) self->xfer.txData[i] = 0xFF;

      self->pixels = self->xfer.txData + BUFFER_BYTES(n);
      self->numLEDs = n;
      DotStar_clear(self);
    } else {
      self->numLEDs = 0;
    }
}


void DotStar_spiInit(DotStar_t *self)
{
	self->xfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 ;

    DSPI_MasterInit(self->spi, &self->masterConfig, BOARD_GetSpiClock(0));
    DSPI_MasterTransferCreateHandle(self->spi, &self->spiHandle, DotStar_spiTransferCallback, self);
}



void DotStar_spiEnd(DotStar_t *self)
{
    DSPI_Deinit(self->spi);
}


/* ISSUE DATA TO LED STRIP -------------------------------------------------

  Although the LED driver has an additional per-pixel 5-bit brightness
  setting, it is NOT used or supported here because it's a brain-dead
  misfeature that's counter to the whole point of Dot Stars, which is to
  have a much faster PWM rate than NeoPixels.  It gates the high-speed
  PWM output through a second, much slower PWM (about 400 Hz), rendering
  it useless for POV.  This brings NOTHING to the table that can't be
  already handled better in one's sketch code.  If you really can't live
  without this abomination, you can fork the library and add it for your
  own use, but any pull requests for this will NOT be merged, nuh uh!
*/

void DotStar_show(DotStar_t *self)
{
  if(!self->xfer.txData || !self->isFinished) return;

  uint16_t b16 = (uint16_t)self->brightness; // Type-convert for fixed-point math

  for (uint16_t n=0; n < self->numLEDs; n++)
  {
    // Pixel Start
    self->xfer.txData[START_BYTES + 4*n] = 0xFF;
    for(uint8_t i=0; i<3; i++)
      self->xfer.txData[START_BYTES + 4*n + 1 + i] = self->brightness ?
            (self->pixels[3*n + i]* b16) >> 8 : self->pixels[3*n + i];
  }

  self->isFinished = FALSE;
  DSPI_MasterTransferNonBlocking(self->spi, &self->spiHandle, &self->xfer);

  //DSPI_MasterTransferBlocking(self->spi, &self->xfer);
  //self->isFinished = TRUE;
}



void DotStar_clear(DotStar_t *self)
{

	FLib_MemSet(self->pixels, 0, (self->rOffset == self->gOffset) ?
     self->numLEDs + ((self->numLEDs + 3) / 4) : // MONO: 10 bits/pixel
     self->numLEDs * 3);

}

// Set global brightness 0-255
void DotStar_setBrightness(DotStar_t *self, uint8_t b)
{
  self->brightness = b + 1;
}



void DotStar_setPixelColorRGB(DotStar_t *self, uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  if(n < self->numLEDs) {
    uint8_t *p = &self->pixels[n * 3];
    p[self->rOffset] = r;
    p[self->gOffset] = g;
    p[self->bOffset] = b;
  }
}


// Set pixel color, 'packed' RGB value (0x000000 - 0xFFFFFF)
void DotStar_setPixelColor(DotStar_t *self, uint16_t n, uint32_t c) {
  if(n < self->numLEDs) {
    uint8_t *p = &self->pixels[n * 3];
    p[self->rOffset] = (uint8_t)(c >> 16);
    p[self->gOffset] = (uint8_t)(c >>  8);
    p[self->bOffset] = (uint8_t)c;
  }
}


uint32_t DotStar_color(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}


uint32_t DotStar_getPixelColor(DotStar_t *self, uint16_t n)
{
    if(n >= self->numLEDs) return 0;
    uint8_t *p = &self->pixels[n * 3];
    return ((uint32_t)p[self->rOffset] << 16) |
           ((uint32_t)p[self->gOffset] <<  8) |
            (uint32_t)p[self->bOffset];
}


uint16_t DotStar_numPixels(DotStar_t *self)
{
    return self->numLEDs;
}


uint8_t DotStar_getBrightness(DotStar_t *self)
{
    return self->brightness -1;
}

uint8_t *DotStar_getPixels(DotStar_t *self)
{
    return self->pixels;
}
