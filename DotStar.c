#include "DotStar.h"
#include "MemManager.h"

void DotStar_updateLength(uint16_t n);;
void DotStar_spiEnd(DotStar_t &self);

void DotStar_spiTransferCallback(SPI_Type *base, dspi_slave_handle_t *handle,
        status_t status, void *userData)
{
    DotStar_t *self = (DotStar_t*)userData;

}

DotStar_t DotStar_create(uint16_t num, uint8_t o, SPI_Type spi, dspi_which_pcs_t cs)
{
    DotStar ret;
    ret.numLEDs = num;
    ret.brightness = 0;
    ret.pixels = NULL;
    ret.rOffset = o & 3;
    ret.gOffset = (o >> 2) & 3;
    ret.bOffset = (o >> 4) & 3;

    DotStar_updateLength(n);
    // get default config
    SPI_MasterGetDefaultConfig(&ret.masterConfig);
    ret.masterConfig.whichPcs = cs;
    //store spi type
    ret.spi = spi;
    return ret;
}


void DotStar_deinit(DotStar_t *self)
{
     if(self->pixels)
        MEM_BufferFree(self->pixels);

     DotStar_spiEnd(self);
}

void DotStar_spiEnd(DotStar_t *self)
{
    //todo
}

void DotStar_spiInit(DotStar_t *self)
{
    DSPI_MasterInit(self->spi, &self->masterConfig);
    DSPI_MasterTransferCreateHandle(self->spi, &self->spiHandle, DotStar_spiTransferCallback, self);
}

void DotStar_begin(DotStar_t *self)
{
    DotStar_spiInit(self);
}

void DotStar_clear(DotStar_t *self)
{
    //todo
}


void DotStar_setBrightness(uint8_t);                 // Set global brightness 0-255
void DotStar_setPixelColor(uint16_t n, uint32_t c);
void DotStar_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);



void DotStar_show(void)
{
    //TODO
}

void DotStar_updateLength(DotStar_t *self, uint16_t n)
{
    if(self->pixels) MEM_BufferFree(self->pixels);
    uint16_t bytes = (self->rOffset == self->gOffset) ?
      n + ((n + 3) / 4) : // MONO: 10 bits/pixel, round up to next byte
      n * 3;              // COLOR: 3 bytes/pixel
    if((self->pixels = (uint8_t *)MEM_BufferAlloc(bytes)) {
      self->numLEDs = n;
      DotStar_clear(self);
    } else {
      self->numLEDs = 0;
    }
}


uint32_t DotStar_color(DotStar_t *self, uint8_t r, uint8_t g, uint8_t b)
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
    return self->numPixels;
}


uint8_t DotStar_getBrightness(DotStar_t *self)
{
    return self->brightness -1;
}

uint8_t *DotStar_getPixels(DotStar_t *self)
{
    return self->pixels;
}
