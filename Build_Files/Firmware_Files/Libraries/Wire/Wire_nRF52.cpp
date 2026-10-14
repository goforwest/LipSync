/*
 * TWI/I2C library for nRF5x
 * Copyright (c) 2015 Arduino LLC. All rights reserved.
 * Copyright (c) 2016 Sandeep Mistry All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if defined(NRF52) || defined(NRF52_SERIES)

extern "C" {
#include <string.h>
}

#include <Arduino.h>
#include <wiring_private.h>

#include "Wire.h"

#if defined(ARDUINO_Seeed_XIAO_nRF52840) || defined(ARDUINO_Seeed_XIAO_nRF52840_Sense) || defined(ARDUINO_Seeed_XIAO_nRF52840_Plus) || defined(ARDUINO_Seeed_XIAO_nRF52840_Sense_Plus) || defined(ARDUINO_TRACKER_T1000_E_LORAWAN)
#include <Adafruit_TinyUSB.h> // for Serial
#elif defined(ARDUINO_WIO_TRACKER_1110)
#else
#error "Unsupported board"
#endif

static volatile uint32_t* pincfg_reg(uint32_t pin)
{
  NRF_GPIO_Type * port = nrf_gpio_pin_port_decode(&pin);
  return &port->PIN_CNF[pin];
}

// *** LipSync patch (makersmakingchange/LipSync) ***
// The stock driver spins on TWIM event registers with no bound. If the bus
// wedges (a sensor browned-out mid-transaction holding SDA low, a damaged
// interface cable), the event never fires and readPressure()/getEvent() never
// return — the whole main loop hangs until the hardware watchdog resets the
// device (30 s of dead USB). Every wait below is bounded by
// WIRE_TRANSACTION_TIMEOUT_MS; on timeout the bus is recovered (bounded STOP,
// then the NXP I2C bus-clear: up to 9 SCL clocks to release a slave stuck
// mid-byte) and the call reports failure, which the firmware's sensor-health
// layer turns into "sensor unhealthy" instead of a hang. 50 ms is ~25x the
// duration of a full 100 kHz transaction, so healthy buses are unaffected.
const uint32_t WIRE_TRANSACTION_TIMEOUT_MS = 50;

// Bus-held reporting (LipSync patch): a transaction that hits the bounded
// timeout + recovery path stamps the time; busHealthy() stays false for
// WIRE_BUS_HOLD_DEBOUNCE_MS afterwards. Consumers that back off while the bus
// is held naturally re-probe at this cadence (their first skipped-period
// transaction after the debounce either succeeds — bus healthy again — or
// times out and re-stamps, self-limiting the cost to ~one recovery per window).
const uint32_t WIRE_BUS_HOLD_DEBOUNCE_MS = 1000;
static volatile unsigned long wireLastRecoveryMs = 0;
static volatile uint32_t wireRecoveryCount = 0;

bool TwoWire::busHealthy()
{
  return wireRecoveryCount == 0 || (millis() - wireLastRecoveryMs) > WIRE_BUS_HOLD_DEBOUNCE_MS;
}

uint32_t TwoWire::recoveryCount()
{
  return wireRecoveryCount;
}

// Waits for *eventReg (or EVENTS_ERROR, which callers handle). Returns true
// when the event fired; false on timeout.
static bool wireWaitEvent(NRF_TWIM_Type * twim, volatile uint32_t * eventReg, uint32_t timeoutMs)
{
  if (*eventReg) { return true; }
  uint32_t start = millis();
  while (!*eventReg)
  {
    if (twim->EVENTS_ERROR) { return true; }  // caller's error path handles it
    if (millis() - start >= timeoutMs) { return false; }
  }
  return true;
}

// Recovers a wedged TWIM: bounded STOP attempt, disconnect + disable the
// peripheral, run the I2C bus-clear on the pins as GPIO, then re-attach and
// re-enable for the next transaction.
static void wireRecoverBus(NRF_TWIM_Type * twim, uint32_t pinSDA, uint32_t pinSCL)
{
  wireLastRecoveryMs = millis();  // Stamp the bus-held window (see busHealthy())
  wireRecoveryCount++;
  // A clean stop first — on a wedged bus even STOP may never complete, so bound it too
  twim->TASKS_STOP = 0x1UL;
  uint32_t start = millis();
  while (!twim->EVENTS_STOPPED && millis() - start < WIRE_TRANSACTION_TIMEOUT_MS);
  twim->EVENTS_STOPPED = 0x0UL;
  twim->EVENTS_ERROR = 0x0UL;
  twim->ERRORSRC = twim->ERRORSRC;  // error sources are write-1-to-clear

  // Release the pins and disable the peripheral so they can be driven as GPIO
  twim->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);
  twim->PSEL.SCL = TWIM_PSEL_SCL_CONNECT_Disconnected;
  twim->PSEL.SDA = TWIM_PSEL_SDA_CONNECT_Disconnected;

  // NXP I2C-bus specification, 3.1.16 "bus clear": clock SCL up to 9 times so
  // a slave stuck mid-byte lets go of SDA. Both pins are open-drain with
  // pull-up, the same drive TwoWire::begin() configures.
  nrf_gpio_cfg(pinSDA, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT,
               NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg(pinSCL, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT,
               NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);

  for (uint8_t i = 0; i < 9; i++)
  {
    nrf_gpio_pin_clear(pinSCL);
    delayMicroseconds(5);
    nrf_gpio_pin_set(pinSCL);
    delayMicroseconds(5);
    if (nrf_gpio_pin_read(pinSDA)) { break; }  // SDA released — the bus is free
  }

  // Re-attach and re-enable the TWIM for the next transaction
  twim->PSEL.SCL = pinSCL;
  twim->PSEL.SDA = pinSDA;
  twim->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);
}
// *** end LipSync patch ***

TwoWire::TwoWire(NRF_TWIM_Type * p_twim, NRF_TWIS_Type * p_twis, IRQn_Type IRQn, uint8_t pinSDA, uint8_t pinSCL)
{
  this->_p_twim = p_twim;
  this->_p_twis = p_twis;
  this->_IRQn = IRQn;
  this->_uc_pinSDA = g_ADigitalPinMap[pinSDA];
  this->_uc_pinSCL = g_ADigitalPinMap[pinSCL];
  transmissionBegun = false;
}

void TwoWire::begin(void) {
  //Main Mode
  master = true;

  *pincfg_reg(_uc_pinSCL) = ((uint32_t)GPIO_PIN_CNF_DIR_Input         << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);

  *pincfg_reg(_uc_pinSDA) = ((uint32_t)GPIO_PIN_CNF_DIR_Input         << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);

  _p_twim->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K100;
  _p_twim->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);
  _p_twim->PSEL.SCL = _uc_pinSCL;
  _p_twim->PSEL.SDA = _uc_pinSDA;

  NVIC_ClearPendingIRQ(_IRQn);
  NVIC_SetPriority(_IRQn, 3);
  NVIC_EnableIRQ(_IRQn);
}

void TwoWire::begin(uint8_t address) {
  //Secondary mode
  master = false;

  *pincfg_reg(_uc_pinSCL) = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);

  *pincfg_reg(_uc_pinSDA) = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                          | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);

  _p_twis->ADDRESS[0] = address;
  _p_twis->CONFIG = TWIS_CONFIG_ADDRESS0_Msk;
  _p_twis->PSEL.SCL = _uc_pinSCL;
  _p_twis->PSEL.SDA = _uc_pinSDA;

  _p_twis->ORC = 0xff;

  _p_twis->INTENSET = TWIS_INTEN_STOPPED_Msk | TWIS_INTEN_ERROR_Msk | TWIS_INTEN_WRITE_Msk | TWIS_INTEN_READ_Msk;

  NVIC_ClearPendingIRQ(_IRQn);
  NVIC_SetPriority(_IRQn, 3);
  NVIC_EnableIRQ(_IRQn);

  _p_twis->ENABLE = (TWIS_ENABLE_ENABLE_Enabled << TWIS_ENABLE_ENABLE_Pos);
}

void TwoWire::setClock(uint32_t baudrate) {
  if (master) {
    _p_twim->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);

    uint32_t frequency;

    if (baudrate <= 100000)
    {
      frequency = TWIM_FREQUENCY_FREQUENCY_K100;
    }
    else if (baudrate <= 250000)
    {
      frequency = TWIM_FREQUENCY_FREQUENCY_K250;
    }
    else
    {
      frequency = TWIM_FREQUENCY_FREQUENCY_K400;
    }

    _p_twim->FREQUENCY = frequency;
    _p_twim->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);
  }
}

void TwoWire::setPins(uint8_t pinSDA, uint8_t pinSCL)
{
    this->_uc_pinSDA = g_ADigitalPinMap[pinSDA];
    this->_uc_pinSCL = g_ADigitalPinMap[pinSCL];
}

void TwoWire::end() {
  if (master)
  {
    _p_twim->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);
  }
  else
  {
    _p_twis->ENABLE = (TWIS_ENABLE_ENABLE_Disabled << TWIS_ENABLE_ENABLE_Pos);
  }
}

uint8_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool stopBit)
{
  if(quantity == 0)
  {
    return 0;
  }

  size_t byteRead = 0;
  rxBuffer.clear();

  _p_twim->ADDRESS = address;

  _p_twim->TASKS_RESUME = 0x1UL;
  _p_twim->RXD.PTR = (uint32_t)rxBuffer._aucBuffer;
  _p_twim->RXD.MAXCNT = quantity;
  _p_twim->TASKS_STARTRX = 0x1UL;

  if (!wireWaitEvent(_p_twim, &_p_twim->EVENTS_RXSTARTED, WIRE_TRANSACTION_TIMEOUT_MS))
  {
    wireRecoverBus(_p_twim, _uc_pinSDA, _uc_pinSCL);
    return 0;
  }
  _p_twim->EVENTS_RXSTARTED = 0x0UL;

  if (!wireWaitEvent(_p_twim, &_p_twim->EVENTS_LASTRX, WIRE_TRANSACTION_TIMEOUT_MS))
  {
    wireRecoverBus(_p_twim, _uc_pinSDA, _uc_pinSCL);
    return 0;
  }
  _p_twim->EVENTS_LASTRX = 0x0UL;

  if (stopBit || _p_twim->EVENTS_ERROR)
  {
    _p_twim->TASKS_STOP = 0x1UL;
    if (!wireWaitEvent(_p_twim, &_p_twim->EVENTS_STOPPED, WIRE_TRANSACTION_TIMEOUT_MS))
    {
      wireRecoverBus(_p_twim, _uc_pinSDA, _uc_pinSCL);
      return 0;
    }
    _p_twim->EVENTS_STOPPED = 0x0UL;
  }
  else
  {
    _p_twim->TASKS_SUSPEND = 0x1UL;
    if (!wireWaitEvent(_p_twim, &_p_twim->EVENTS_SUSPENDED, WIRE_TRANSACTION_TIMEOUT_MS))
    {
      wireRecoverBus(_p_twim, _uc_pinSDA, _uc_pinSCL);
      return 0;
    }
    _p_twim->EVENTS_SUSPENDED = 0x0UL;
  }

  if (_p_twim->EVENTS_ERROR)
  {
    _p_twim->EVENTS_ERROR = 0x0UL;
  }

  byteRead = rxBuffer._iHead = _p_twim->RXD.AMOUNT;

  return byteRead;
}

uint8_t TwoWire::requestFrom(uint8_t address, size_t quantity)
{
  return requestFrom(address, quantity, true);
}

void TwoWire::beginTransmission(uint8_t address) {
  // save address of target and clear buffer
  txAddress = address;
  txBuffer.clear();

  transmissionBegun = true;
}

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
uint8_t TwoWire::endTransmission(bool stopBit)
{
  transmissionBegun = false ;

  // Start I2C transmission
  _p_twim->ADDRESS = txAddress;

  // just in case twi is stopped by bus error such as secondary device reset/stalled without replying ACK/NACK
  _p_twim->EVENTS_STOPPED = 0x0UL;
  _p_twim->TASKS_RESUME = 0x1UL;

  _p_twim->TXD.PTR = (uint32_t)txBuffer._aucBuffer;
  _p_twim->TXD.MAXCNT = txBuffer.available();

  _p_twim->TASKS_STARTTX = 0x1UL;

  if (!wireWaitEvent(_p_twim, &_p_twim->EVENTS_TXSTARTED, WIRE_TRANSACTION_TIMEOUT_MS))
  {
    wireRecoverBus(_p_twim, _uc_pinSDA, _uc_pinSCL);
    return 4;  // Other error
  }
  _p_twim->EVENTS_TXSTARTED = 0x0UL;

  if (txBuffer.available()) {
    if (!wireWaitEvent(_p_twim, &_p_twim->EVENTS_LASTTX, WIRE_TRANSACTION_TIMEOUT_MS))
    {
      wireRecoverBus(_p_twim, _uc_pinSDA, _uc_pinSCL);
      return 4;  // Other error
    }
  }
  _p_twim->EVENTS_LASTTX = 0x0UL;

  if (stopBit || _p_twim->EVENTS_ERROR)
  {
    _p_twim->TASKS_STOP = 0x1UL;
    if (!wireWaitEvent(_p_twim, &_p_twim->EVENTS_STOPPED, WIRE_TRANSACTION_TIMEOUT_MS))
    {
      wireRecoverBus(_p_twim, _uc_pinSDA, _uc_pinSCL);
      return 4;  // Other error
    }
    _p_twim->EVENTS_STOPPED = 0x0UL;
  }
  else
  {
    _p_twim->TASKS_SUSPEND = 0x1UL;
    if (!wireWaitEvent(_p_twim, &_p_twim->EVENTS_SUSPENDED, WIRE_TRANSACTION_TIMEOUT_MS))
    {
      wireRecoverBus(_p_twim, _uc_pinSDA, _uc_pinSCL);
      return 4;  // Other error
    }
    _p_twim->EVENTS_SUSPENDED = 0x0UL;
  }

  if (_p_twim->EVENTS_ERROR)
  {
    _p_twim->EVENTS_ERROR = 0x0UL;

    uint32_t error = _p_twim->ERRORSRC;

    _p_twim->ERRORSRC = error;

    if (error == TWIM_ERRORSRC_ANACK_Msk)
    {
      return 2;
    }
    else if (error == TWIM_ERRORSRC_DNACK_Msk)
    {
      return 3;
    }
    else
    {
      return 4;
    }
  }

  return 0;
}

uint8_t TwoWire::endTransmission()
{
  return endTransmission(true);
}

size_t TwoWire::write(uint8_t ucData)
{
  // No writing, without begun transmission or a full buffer
  if ( !transmissionBegun || txBuffer.isFull() )
  {
    return 0 ;
  }

  txBuffer.store_char( ucData ) ;

  return 1 ;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  //Try to store all data
  for(size_t i = 0; i < quantity; ++i)
  {
    //Return the number of data stored, when the buffer is full (if write return 0)
    if(!write(data[i]))
      return i;
  }

  //All data stored
  return quantity;
}

int TwoWire::available(void)
{
  return rxBuffer.available();
}

int TwoWire::read(void)
{
  return rxBuffer.read_char();
}

int TwoWire::peek(void)
{
  return rxBuffer.peek();
}

void TwoWire::flush(void)
{
  // Do nothing, use endTransmission(..) to force
  // data transfer.
}

void TwoWire::onReceive(void(*function)(int))
{
  onReceiveCallback = function;
}

void TwoWire::onRequest(void(*function)(void))
{
  onRequestCallback = function;
}

void TwoWire::onService(void)
{
  if (_p_twis->EVENTS_WRITE)
  {
    _p_twis->EVENTS_WRITE = 0x0UL;

    receiving = true;

    rxBuffer.clear();

    _p_twis->RXD.PTR = (uint32_t)rxBuffer._aucBuffer;
    _p_twis->RXD.MAXCNT = sizeof(rxBuffer._aucBuffer);

    _p_twis->TASKS_PREPARERX = 0x1UL;
  }

  if (_p_twis->EVENTS_READ)
  {
    _p_twis->EVENTS_READ = 0x0UL;

    receiving = false;
    transmissionBegun = true;

    txBuffer.clear();

    if (onRequestCallback)
    {
      onRequestCallback();
    }

    transmissionBegun = false;

    _p_twis->TXD.PTR = (uint32_t)txBuffer._aucBuffer;
    _p_twis->TXD.MAXCNT = txBuffer.available();

    _p_twis->TASKS_PREPARETX = 0x1UL;
  }

  if (_p_twis->EVENTS_STOPPED)
  {
    _p_twis->EVENTS_STOPPED = 0x0UL;

    if (receiving)
    {
      int rxAmount = _p_twis->RXD.AMOUNT;

      rxBuffer._iHead = rxAmount;

      if (onReceiveCallback)
      {
        onReceiveCallback(rxAmount);
      }
    }
  }

  if (_p_twis->EVENTS_ERROR)
  {
    _p_twis->EVENTS_ERROR = 0x0UL;

    uint32_t error = _p_twis->ERRORSRC;
    _p_twis->ERRORSRC = error;

    _p_twis->TASKS_STOP = 0x1UL;
  }
}

#if WIRE_INTERFACES_COUNT > 0
TwoWire Wire(NRF_TWIM0, NRF_TWIS0, SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn, PIN_WIRE_SDA, PIN_WIRE_SCL);

extern "C"
{
  void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void)
  {
    #if CFG_SYSVIEW
    SEGGER_SYSVIEW_RecordEnterISR();
    #endif

    Wire.onService();

    #if CFG_SYSVIEW
    SEGGER_SYSVIEW_RecordExitISR();
    #endif
  }
}
#endif

#if WIRE_INTERFACES_COUNT > 1
TwoWire Wire1(NRF_TWIM1, NRF_TWIS1, SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn, PIN_WIRE1_SDA, PIN_WIRE1_SCL);

extern "C"
{
  void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler(void)
  {
    #if CFG_SYSVIEW
    SEGGER_SYSVIEW_RecordEnterISR();
    #endif

    Wire1.onService();

    #if CFG_SYSVIEW
    SEGGER_SYSVIEW_RecordExitISR();
    #endif
  }
}
#endif

#endif // NRF52_SERIES
