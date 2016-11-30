/*------------------------------------------------------------------------
  This file is part of the Adafruit Dot Star library.

  Adafruit Dot Star is free software: you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  Adafruit Dot Star is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with DotStar.  If not, see <http://www.gnu.org/licenses/>.
  ------------------------------------------------------------------------*/

#ifndef _ADAFRUIT_DOT_STAR_H_
#define _ADAFRUIT_DOT_STAR_H_

#include "fsl_dspi.h"
#include "EmbeddedTypes.h"

// Color-order flag for LED pixels (optional extra parameter to constructor):
// Bits 0,1 = R index (0-2), bits 2,3 = G index, bits 4,5 = B index
#define DOTSTAR_RGB (0 | (1 << 2) | (2 << 4))
#define DOTSTAR_RBG (0 | (2 << 2) | (1 << 4))
#define DOTSTAR_GRB (1 | (0 << 2) | (2 << 4))
#define DOTSTAR_GBR (2 | (0 << 2) | (1 << 4))
#define DOTSTAR_BRG (1 | (2 << 2) | (0 << 4))
#define DOTSTAR_BGR (2 | (1 << 2) | (0 << 4))
#define DOTSTAR_MONO 0 // Single-color strip WIP DO NOT USE YET

typedef struct DotStar_obj
{
    uint16_t numLEDs;                                // Number of pixels
    uint8_t
      brightness,                             // Global brightness setting
     *pixels,                                 // LED RGB values (3 bytes ea.)
      rOffset,                                // Index of red in 3-byte pixel
      gOffset,                                // Index of green byte
      bOffset;                                // Index of blue byte

      //SPI settings
      dspi_master_handle_t spiHandle;
      dspi_master_config_t masterConfig;
      SPI_Type *spi;
      uint8_t isFinished;
      dspi_transfer_t xfer;
} DotStar_t;

// construct/deconstruct
DotStar_t DotStar_create(uint16_t num, uint8_t o, SPI_Type *spi);
void DotStar_deinit(DotStar_t *self);

void DotStar_begin(DotStar_t *self);
void DotStar_updateLength(DotStar_t *self, uint16_t n);
void DotStar_show(DotStar_t *self);
void DotStar_clear(DotStar_t *self);
// Set global brightness 0-255
void DotStar_setBrightness(DotStar_t *self, uint8_t b);
void DotStar_setPixelColorRGB(DotStar_t *self, uint16_t n, uint8_t r, uint8_t g, uint8_t b);
void DotStar_setPixelColor(DotStar_t *self, uint16_t n, uint32_t c);
uint32_t DotStar_getPixelColor(DotStar_t *self, uint16_t n);
uint16_t DotStar_numPixels(DotStar_t *self);
uint8_t DotStar_getBrightness(DotStar_t *self);
uint8_t *DotStar_getPixels(DotStar_t *self);

// static
uint32_t DotStar_color(uint8_t r, uint8_t g, uint8_t b);

#endif // _ADAFRUIT_DOT_STAR_H_
