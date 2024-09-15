/***************************************************
  This is a library for the ST7789 IPS SPI display.

  Written by Ananev Ilya.
 ****************************************************/

#include "Arduino_ST7789.h"

static const uint8_t PROGMEM
    cmd_240x320[] = {                // Init commands for 7789 screens
    9,                              //  9 commands in list:
    ST7789_SWRESET,   ST_CMD_DELAY, //  1: Software reset, no args, w/delay
      150,                          //     ~150 ms delay
    ST7789_SLPOUT ,   ST_CMD_DELAY, //  2: Out of sleep mode, no args, w/delay
      10,                          //      10 ms delay
    ST7789_COLMOD , 1+ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
      0x55,                         //     16-bit color
      10,                           //     10 ms delay
    ST7789_MADCTL , 1,              //  4: Mem access ctrl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    ST7789_CASET  , 4,              //  5: Column addr set, 4 args, no delay:
      0x00,
      0,        //     XSTART = 0
      0,
      240,  //     XEND = 240
    ST7789_RASET  , 4,              //  6: Row addr set, 4 args, no delay:
      0x00,
      0,             //     YSTART = 0
      320>>8,
      320&0xFF,  //     YEND = 320
    ST7789_INVOFF  ,   ST_CMD_DELAY,  //  7: hack
      10,
    ST7789_NORON  ,   ST_CMD_DELAY, //  8: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST7789_DISPON ,   ST_CMD_DELAY, //  9: Main screen turn on, no args, delay
      10 };                          //    10 ms delay

inline uint16_t swapcolor(uint16_t x)
{
  return (x << 11) | (x & 0x07E0) | (x >> 11);
}

static SPISettings mySPISettings;

#define SPI_BEGIN_TRANSACTION() \
  if (_hwSPI)                   \
  _spi->beginTransaction(mySPISettings)
#define SPI_END_TRANSACTION() \
  if (_hwSPI)                 \
  _spi->endTransaction()

// Constructor when using software SPI.  All output pins are configurable.
Arduino_ST7789::Arduino_ST7789(int8_t dc, int8_t rst, int8_t sid, int8_t sclk, int8_t cs)
{
  _cs = cs;
  _dc = dc;
  _sid = sid;
  _sclk = sclk;
  _rst = rst;
  _hwSPI = false;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Arduino_ST7789::Arduino_ST7789(int8_t dc, int8_t rst, int8_t cs)
{
  _cs = cs;
  _dc = dc;
  _rst = rst;
  _hwSPI = true;
  _sid = _sclk = -1;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
Arduino_ST7789::Arduino_ST7789(int8_t dc, int8_t rst, int8_t cs, SPIClass *spiClass)
{
  _spi = spiClass;
  _cs = cs;
  _dc = dc;
  _rst = rst;
  _hwSPI = true;
  _sid = _sclk = -1;
}

inline void Arduino_ST7789::spiwrite(uint8_t c)
{

  // Serial.println(c, HEX);

  if (_hwSPI)
  {
    _spi->transfer(c);
  }
  else
  {

    // Fast SPI bitbang swiped from LPD8806 library
    for (uint8_t bit = 0x80; bit; bit >>= 1)
    {
      digitalWrite(_sclk, LOW);
      if (c & bit)
        digitalWrite(_sid, HIGH);
      else
        digitalWrite(_sid, LOW);
      digitalWrite(_sclk, HIGH);
    }
  }
}

void Arduino_ST7789::writecommand(uint8_t c)
{

  DC_LOW();
  CS_LOW();
  SPI_BEGIN_TRANSACTION();

  spiwrite(c);

  CS_HIGH();
  SPI_END_TRANSACTION();
}

void Arduino_ST7789::writedata(uint8_t c)
{
  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  spiwrite(c);

  CS_HIGH();
  SPI_END_TRANSACTION();
}

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void Arduino_ST7789::displayInit(const uint8_t *addr)
{

  uint8_t numCommands, numArgs;
  uint16_t ms;
  //<-----------------------------------------------------------------------------------------
  DC_HIGH();

  if (!_hwSPI)
  {
    pinMode(_sclk, OUTPUT);
    digitalWrite(_sclk, HIGH);
  }
  //<-----------------------------------------------------------------------------------------

  numCommands = pgm_read_byte(addr++); // Number of commands to follow
  while (numCommands--)
  { // For each command...
    // Serial.printf("Command: 0x%x\n", *addr);
    writecommand(pgm_read_byte(addr++)); //   Read, issue command
    numArgs = pgm_read_byte(addr++);     //   Number of args to follow
    ms = numArgs & ST_CMD_DELAY;         //   If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;            //   Mask out delay bit
    while (numArgs--)
    { //   For each argument...
      // Serial.printf("\t\tParameter: 0x%x\n", *addr);
      writedata(pgm_read_byte(addr++)); //     Read, issue argument
    }

    if (ms)
    {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if (ms == 255)
        ms = 500; // If 255, delay for 500 ms
      // Serial.printf("\t\tdelay: %d\n", ms);
      delay(ms);
    }
  }
}

// Initialization code common to all ST7789 displays
void Arduino_ST7789::commonInit(const uint8_t *cmdList)
{
  _ystart = _xstart = 0;
  _colstart = _rowstart = 0; // May be overridden in init func

  pinMode(_dc, OUTPUT);
  if (_cs)
  {
    pinMode(_cs, OUTPUT);
  }

  if (_hwSPI)
  { // Using hardware SPI
    _spi->begin();
    mySPISettings = SPISettings(80000000, MSBFIRST, SPI_MODE0);
  }
  else
  {
    pinMode(_sclk, OUTPUT);
    pinMode(_sid, OUTPUT);
    digitalWrite(_sclk, LOW);
    digitalWrite(_sid, LOW);
  }

  // toggle RST low to reset; CS low so it'll listen to us
  CS_LOW();
  if (_rst != -1)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(200);
    digitalWrite(_rst, LOW);
    delay(200);
    digitalWrite(_rst, HIGH);
    delay(200);
  }

  if (cmdList)
    displayInit(cmdList);
}

void Arduino_ST7789::setRotation(uint8_t m)
{
  writecommand(ST7789_MADCTL);
  int rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     writedata(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);

     _xstart = _colstart;
     _ystart = _rowstart;
     break;
   case 1:
     writedata(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);

     _ystart = _colstart;
     _xstart = _rowstart;
     break;
  case 2:
     writedata(ST7789_MADCTL_RGB);
 
     _xstart = _colstart;
     _ystart = _rowstart;
     break;

   case 3:
     writedata(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);

     _ystart = _colstart;
     _xstart = _rowstart;
     break;
  }
}

void Arduino_ST7789::setMADCTL(uint8_t data)
{
  writecommand(ST7789_MADCTL);
  writedata(data);
}

void Arduino_ST7789::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{

  uint16_t x_start = x0 + _xstart, x_end = x1 + _xstart;
  uint16_t y_start = y0 + _ystart, y_end = y1 + _ystart;

  writecommand(ST7789_CASET); // Column addr set
  writedata(x_start >> 8);
  writedata(x_start & 0xFF); // XSTART
  writedata(x_end >> 8);
  writedata(x_end & 0xFF); // XEND

  writecommand(ST7789_RASET); // Row addr set
  writedata(y_start >> 8);
  writedata(y_start & 0xFF); // YSTART
  writedata(y_end >> 8);
  writedata(y_end & 0xFF); // YEND

  writecommand(ST7789_RAMWR); // write to RAM
}

void Arduino_ST7789::pushColor(uint16_t color)
{
  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  spiwrite(color >> 8);
  spiwrite(color);

  CS_HIGH();
  SPI_END_TRANSACTION();
}

/* -- Pushing data with three functions together -- */
void Arduino_ST7789::startWrite()
{
  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();
}

void Arduino_ST7789::endWrite()
{
  CS_HIGH();
  SPI_END_TRANSACTION();
}

void Arduino_ST7789::pushData(uint16_t color)
{
  spiwrite(color >> 8);
  spiwrite(color);
}

void Arduino_ST7789::pushDataMultiple(uint16_t *color, size_t size)
{
  _spi->writePixels(color, size * 2);
}
/* -- xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx -- */

void Arduino_ST7789::drawPixel(int16_t x, int16_t y, uint16_t color)
{

  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
    return;

  setAddrWindow(x, y, x + 1, y + 1);

  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  spiwrite(color >> 8);
  spiwrite(color);

  CS_HIGH();
  SPI_END_TRANSACTION();
}

void Arduino_ST7789::drawFastVLine(int16_t x, int16_t y, int16_t h,
                                   uint16_t color)
{

  // Rudimentary clipping
  if ((x >= _width) || (y >= _height))
    return;
  if ((y + h - 1) >= _height)
    h = _height - y;
  setAddrWindow(x, y, x, y + h - 1);

  uint8_t hi = color >> 8, lo = color;

  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  while (h--)
  {
    spiwrite(hi);
    spiwrite(lo);
  }

  CS_HIGH();
  SPI_END_TRANSACTION();
}

void Arduino_ST7789::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                   uint16_t color)
{

  // Rudimentary clipping
  if ((x >= _width) || (y >= _height))
    return;
  if ((x + w - 1) >= _width)
    w = _width - x;
  setAddrWindow(x, y, x + w - 1, y);

  uint8_t hi = color >> 8, lo = color;

  SPI_BEGIN_TRANSACTION();
  DC_HIGH();
  CS_LOW();

  while (w--)
  {
    spiwrite(hi);
    spiwrite(lo);
  }

  CS_HIGH();
  SPI_END_TRANSACTION();
}

void Arduino_ST7789::fillScreen(uint16_t color)
{
  fillRect(0, 0, _width, _height, color);
}

// fill a rectangle
void Arduino_ST7789::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                              uint16_t color)
{

  // rudimentary clipping (drawChar w/big text requires this)
  if ((x >= _width) || (y >= _height))
    return;
  if ((x + w - 1) >= _width)
    w = _width - x;
  if ((y + h - 1) >= _height)
    h = _height - y;

  setAddrWindow(x, y, x + w - 1, y + h - 1);

  uint8_t hi = color >> 8, lo = color;

  SPI_BEGIN_TRANSACTION();

  DC_HIGH();
  CS_LOW();
  for (y = h; y > 0; y--)
  {
    for (x = w; x > 0; x--)
    {
      spiwrite(hi);
      spiwrite(lo);
    }
  }
  CS_HIGH();
  SPI_END_TRANSACTION();
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Arduino_ST7789::Color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void Arduino_ST7789::invertDisplay(boolean i)
{
  writecommand(i ? ST7789_INVON : ST7789_INVOFF);
}

/******** low level bit twiddling **********/

inline void Arduino_ST7789::CS_HIGH(void)
{
  if (_cs)
  {
#if defined(USE_FAST_IO)
    *csport |= cspinmask;
#else
    digitalWrite(_cs, HIGH);
#endif
  }
}

inline void Arduino_ST7789::CS_LOW(void)
{
  if (_cs)
  {
#if defined(USE_FAST_IO)
    *csport &= ~cspinmask;
#else
    digitalWrite(_cs, LOW);
#endif
  }
}

inline void Arduino_ST7789::DC_HIGH(void)
{
  digitalWrite(_dc, HIGH);
}

inline void Arduino_ST7789::DC_LOW(void)
{
  digitalWrite(_dc, LOW);
}

void Arduino_ST7789::init(uint16_t width, uint16_t height)
{
  pinMode(_dc, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_rst, OUTPUT);

  commonInit(NULL);
  _colstart = ST7789_240x320_XSTART;
  _rowstart = ST7789_240x320_YSTART;
  _height = height;
  _width = width;

  displayInit(cmd_240x320);

  setRotation(2);
}
