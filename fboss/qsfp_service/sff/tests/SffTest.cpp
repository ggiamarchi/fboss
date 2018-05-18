/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <cstdint>
#include <folly/Conv.h>
#include <folly/Memory.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "fboss/qsfp_service/sff/TransceiverImpl.h"
#include "fboss/qsfp_service/sff/QsfpModule.h"

#include <gtest/gtest.h>

using namespace facebook::fboss;
using std::make_unique;

namespace {

/*
 * This is the Wedge Platform Specific Class
 * and contains all the Wedge QSFP Specific Functions
 */
class SffTransceiver : public TransceiverImpl {
 public:
  explicit SffTransceiver(int module) : module_(module) {
    moduleName_ = folly::to<std::string>(module);
  } ;

  /* This function is used to read the SFP EEprom */
  int readTransceiver(int dataAddress, int offset,
                      int len, uint8_t* fieldValue) override;
  /* write to the eeprom (usually to change the page setting) */
  int writeTransceiver(int dataAddress, int offset,
                       int len, uint8_t* fieldValue) override;
  /* This function detects if a SFP is present on the particular port */
  bool detectTransceiver() override;
  /* Returns the name for the port */
  folly::StringPiece getName() override;
  int getNum() const override;

 private:
  int module_;
  std::string moduleName_;
  int page_{0};
};

static uint8_t pageLower[] = {
  0x0d, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x04,
  0x00, 0x00, 0x80, 0xdd, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
};

static uint8_t page0[] = {
  0x0d, 0x10, 0x0c, 0x04, 0x00, 0x00, 0x00, 0x40,
  0x40, 0x02, 0x00, 0x05, 0x67, 0x00, 0x00, 0x32,
  0x00, 0x00, 0x00, 0x00, 0x46, 0x41, 0x43, 0x45,
  0x54, 0x45, 0x53, 0x54, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x07, 0x00, 0x00, 0x00,
  0x46, 0x54, 0x4c, 0x34, 0x31, 0x30, 0x51, 0x45,
  0x32, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x41, 0x20, 0x42, 0x68, 0x07, 0xd0, 0x46, 0x97,
  0x00, 0x01, 0x04, 0xd0, 0x4d, 0x52, 0x45, 0x30,
  0x31, 0x42, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x31, 0x34, 0x30, 0x35,
  0x30, 0x32, 0x20, 0x20, 0x0a, 0x00, 0x00, 0x22,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* These are not supposed to be the same!? */

static uint8_t page3[] = {
  0x4b, 0x00, 0xfb, 0x00, 0x46, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x94, 0x70, 0x6e, 0xf0, 0x86, 0xc4, 0x7b, 0x0c,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


bool SffTransceiver::detectTransceiver() {
  return true;
}

int SffTransceiver::readTransceiver(int dataAddress, int offset,
                                    int len, uint8_t* fieldValue) {
  int read = 0;
  EXPECT_EQ(0x50, dataAddress);
  if (offset < QsfpModule::MAX_QSFP_PAGE_SIZE) {
    read = len;
    if (QsfpModule::MAX_QSFP_PAGE_SIZE - offset < len) {
      read = QsfpModule::MAX_QSFP_PAGE_SIZE - offset;
    }
    memcpy(fieldValue, pageLower + offset, read);
    len -= read;
    offset = QsfpModule::MAX_QSFP_PAGE_SIZE;
  }
  if (len > 0 && offset >= QsfpModule::MAX_QSFP_PAGE_SIZE) {
    uint8_t *dataPage = (page_ == 0) ? page0 : page3;
    offset -= QsfpModule::MAX_QSFP_PAGE_SIZE;
    EXPECT_LE(len + offset, QsfpModule::MAX_QSFP_PAGE_SIZE);
    memcpy(fieldValue + read, dataPage + offset, len);
    read += len;
  }
  return read;
}

int SffTransceiver::writeTransceiver(
    int /*dataAddress*/,
    int offset,
    int len,
    uint8_t* fieldValue) {
  /*
   * This obviously depends on the transceiver parsing code only
   * using the write function to change the page to query.
   * That seems like a reasonable assumption to get this going.
   */
  EXPECT_EQ(offset, 127);
  EXPECT_EQ(len, 1);
  page_ = *fieldValue;
  return len;
}

folly::StringPiece SffTransceiver::getName() {
  return moduleName_;
}

int SffTransceiver::getNum() const {
  return module_;
}


TEST(SffTest, simpleRead) {
  int idx = 1;
  std::unique_ptr<SffTransceiver> qsfpImpl =
    std::make_unique<SffTransceiver>(idx);
  std::unique_ptr<QsfpModule> qsfp =
    std::make_unique<QsfpModule>(std::move(qsfpImpl), 4);
  qsfp->refresh();

  TransceiverInfo info = qsfp->getTransceiverInfo();

  EXPECT_EQ("FACETEST", info.vendor.name);
  EXPECT_EQ(100, info.cable.om3);
  EXPECT_DOUBLE_EQ(3.2989, info.sensor.vcc.value);
  EXPECT_DOUBLE_EQ(31.015625, info.sensor.temp.value);
  EXPECT_DOUBLE_EQ(75.0, info.thresholds.temp.alarm.high);
  EXPECT_DOUBLE_EQ(-5.0, info.thresholds.temp.alarm.low);
  EXPECT_TRUE(info.channels[0].sensors.txBias.flags.alarm.low);
  EXPECT_FALSE(info.channels[1].sensors.txBias.flags.alarm.low);
}

} // namespace facebook::fboss
