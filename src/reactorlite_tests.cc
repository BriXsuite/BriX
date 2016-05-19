#include <gtest/gtest.h>

#include "reactorlite.h"

#include "agent_tests.h"
#include "context.h"
#include "facility_tests.h"


using reactorlite::ReactorLite;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class ReactorLiteTest : public ::testing::Test {
 protected:
  cyclus::TestContext tc;
  ReactorLite* facility;

  virtual void SetUp() {
    facility = new ReactorLite(tc.get());

  }

  virtual void TearDown() {
    delete facility;
  }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ReactorLiteTest, InitialState) {
  // Test things about the initial state of the facility here
  //EXPECT_EQ(0.2, tolerance);

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ReactorLiteTest, Print) {
  EXPECT_NO_THROW(std::string s = facility->str());
  // Test ReactorLite specific aspects of the print method here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ReactorLiteTest, Tick) {
  ASSERT_NO_THROW(facility->Tick());
  // Test ReactorLite specific behaviors of the Tick function here
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TEST_F(ReactorLiteTest, Tock) {
  EXPECT_NO_THROW(facility->Tock());
  // Test ReactorLite specific behaviors of the Tock function here
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Do Not Touch! Below section required for connection with Cyclus
cyclus::Agent* ReactorLiteConstructor(cyclus::Context* ctx) {
  return new ReactorLite(ctx);
}
// Required to get functionality in cyclus agent unit tests library
#ifndef CYCLUS_AGENT_TESTS_CONNECTED
int ConnectAgentTests();
static int cyclus_agent_tests_connected = ConnectAgentTests();
#define CYCLUS_AGENT_TESTS_CONNECTED cyclus_agent_tests_connected
#endif  // CYCLUS_AGENT_TESTS_CONNECTED
INSTANTIATE_TEST_CASE_P(ReactorLite, FacilityTests,
                        ::testing::Values(&ReactorLiteConstructor));
INSTANTIATE_TEST_CASE_P(ReactorLite, AgentTests,
                        ::testing::Values(&ReactorLiteConstructor));
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

















