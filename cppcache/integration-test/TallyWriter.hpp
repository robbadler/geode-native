/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#ifndef GEODE_INTEGRATION_TEST_TALLYWRITER_H_
#define GEODE_INTEGRATION_TEST_TALLYWRITER_H_

#include <geode/CacheableKey.hpp>
#include <geode/CacheWriter.hpp>

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheWriter;
using apache::geode::client::EntryEvent;
using apache::geode::client::Region;
using apache::geode::client::RegionEvent;

class TallyWriter : virtual public CacheWriter {
 private:
  int m_creates;
  int m_updates;
  int m_invalidates;
  int m_destroys;
  bool isWriterInvoke;
  bool isCallbackCalled;
  bool isWriterfailed;
  std::shared_ptr<CacheableKey> m_lastKey;
  std::shared_ptr<Cacheable> m_lastValue;
  std::shared_ptr<CacheableKey> m_callbackArg;

 public:
  TallyWriter()
      : CacheWriter(),
        m_creates(0),
        m_updates(0),
        m_invalidates(0),
        m_destroys(0),
        isWriterInvoke(false),
        isCallbackCalled(false),
        isWriterfailed(false),
        m_lastKey(),
        m_lastValue(),
        m_callbackArg(nullptr) {
    LOG("TallyWriter Constructor called");
  }

  ~TallyWriter() override = default;

  bool beforeCreate(const EntryEvent& event) override {
    m_creates++;
    checkcallbackArg(event);
    LOG("TallyWriter::beforeCreate");
    return !isWriterfailed;
  }

  bool beforeUpdate(const EntryEvent& event) override {
    m_updates++;
    checkcallbackArg(event);
    LOG("TallyWriter::beforeUpdate");
    return !isWriterfailed;
  }

  bool beforeInvalidate(const EntryEvent& event) {
    m_invalidates++;
    checkcallbackArg(event);
    LOG("TallyWriter::beforeInvalidate");
    return !isWriterfailed;
  }

  bool beforeDestroy(const EntryEvent& event) override {
    m_destroys++;
    checkcallbackArg(event);
    LOG("TallyWriter::beforeDestroy");
    return !isWriterfailed;
  }

  bool beforeRegionDestroy(const RegionEvent&) override {
    LOG("TallyWriter::beforeRegionDestroy");
    return !isWriterfailed;
  }

  void close(Region&) override { LOG("TallyWriter::close"); }

  int expectCreates(int expected) {
    int tries = 0;
    while ((m_creates < expected) && (tries < 200)) {
      SLEEP(100);
      tries++;
    }
    return m_creates;
  }

  int getCreates() { return m_creates; }

  int expectUpdates(int expected) {
    int tries = 0;
    while ((m_updates < expected) && (tries < 200)) {
      SLEEP(100);
      tries++;
    }
    return m_updates;
  }

  int getUpdates() { return m_updates; }
  int getInvalidates() { return m_invalidates; }
  int getDestroys() { return m_destroys; }
  void setWriterFailed() { isWriterfailed = true; }
  void setCallBackArg(const std::shared_ptr<CacheableKey>& callbackArg) {
    m_callbackArg = callbackArg;
  }
  void resetWriterInvokation() {
    isWriterInvoke = false;
    isCallbackCalled = false;
  }
  bool isWriterInvoked() { return isWriterInvoke; }
  bool isCallBackArgCalled() { return isCallbackCalled; }
  std::shared_ptr<CacheableKey> getLastKey() { return m_lastKey; }

  std::shared_ptr<Cacheable> getLastValue() { return m_lastValue; }

  void showTallies() {
    char buf[1024];
    sprintf(buf,
            "TallyWriter state: (updates = %d, creates = %d, invalidates = %d, "
            "destroy = %d)",
            getUpdates(), getCreates(), getInvalidates(), getDestroys());
    LOG(buf);
  }
  void checkcallbackArg(const EntryEvent& event) {
    if (!isWriterInvoke) isWriterInvoke = true;
    if (m_callbackArg != nullptr) {
      auto callbkArg =
          std::dynamic_pointer_cast<CacheableKey>(event.getCallbackArgument());
      if (strcmp(m_callbackArg->toString().c_str(),
                 callbkArg->toString().c_str()) == 0) {
        isCallbackCalled = true;
      }
    }
  }
};

#endif  // GEODE_INTEGRATION_TEST_TALLYWRITER_H_
