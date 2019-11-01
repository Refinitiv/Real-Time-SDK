/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "Access/Impl/OmmBaseImplMap.h"
#include <vector>

using namespace thomsonreuters::ema::access;
using namespace std;

#ifdef USING_POLL
class EventFds : public OmmCommonImpl {
public:
  EventFds() {
	_eventFds = new pollfd[2];
	_eventFdsCount = 0;
	_eventFdsCapacity = 2;

	std::vector<int> expected;
	addFd(1); expected.push_back(1);
	addFd(2); expected.push_back(2);
	addFd(3); expected.push_back(3);
	addFd(4); expected.push_back(4);
	addFd(5); expected.push_back(5);
	EXPECT_EQ(_eventFdsCount, 5);
	EXPECT_TRUE(verifyContents(expected));

	{
	  int items[] = {2,3,1,5,4};
	  int size = sizeof items / sizeof items[0];
	  for (int i = 0; i < size; ++i) {
		removeFd(items[i]);
		EXPECT_EQ(_eventFdsCount, size - i - 1);
		expected.erase(std::remove(expected.begin(), expected.end(), items[i]), expected.end());
		EXPECT_TRUE(verifyContents(expected));
	  }
	}

	removeFd(4);					// not there
	EXPECT_EQ(_eventFdsCount, 0);
	EXPECT_TRUE(verifyContents(expected));

	addFd(1); expected.push_back(1);
	addFd(3); expected.push_back(3);
	addFd(2); expected.push_back(2);
	addFd(9); expected.push_back(9);
	EXPECT_EQ(_eventFdsCount, 4);
	EXPECT_TRUE(verifyContents(expected));

	{
	  int items[] = {9,2,3,1};
	  int size = sizeof items / sizeof items[0];
	  for (int i = 0; i < size; ++i) {
		removeFd(items[i]);
		EXPECT_EQ(_eventFdsCount, size - i - 1);
		expected.erase(std::remove(expected.begin(), expected.end(), items[i]), expected.end());
		EXPECT_TRUE(verifyContents(expected));
	  }
	}
  }
private:
  bool verifyContents(std::vector<int> expected) {
	if ( _eventFdsCount != expected.size() ) {
	  cout << "expected size (" << expected.size() << ") does not match _eventFdsCount ("
		   << _eventFdsCount << ")" << endl;
	  return false;
	}
	std::vector<int> actualFds;
	for (int i = 0; i < _eventFdsCount; ++i)
	  actualFds.push_back(_eventFds[i].fd);
	sort(expected.begin(), expected.end());
	sort(actualFds.begin(), actualFds.end());
	if (expected == actualFds)
	  return true;
	else {
	  cout << "expected fds [";
	  std::copy(expected.begin(), expected.end(), std::ostream_iterator<int>(std::cout, " "));
	  cout << "] did not match actual fds [";
	  std::copy(actualFds.begin(), actualFds.end(), std::ostream_iterator<int>(std::cout, " "));
	  cout << ']' << endl;
	  return false;
	}
  }

  // all needed because of pure virtual functions
  ImplementationType getImplType() { return ConsumerEnum; }
  void handleIue(const EmaString&, Int32) {}
  void handleIue(const char*, Int32) {}
  void handleIhe(UInt64, const EmaString&) {}
  void handleIhe(UInt64, const char*) {}
  void handleMee(const char*) {}
  LoggerConfig& getActiveLoggerConfig() {}
  OmmLoggerClient& getOmmLoggerClient() {}
  ErrorClientHandler& getErrorClientHandler() {}
  bool hasErrorClientHandler() const {}
  EmaString& getInstanceName() const {}
  void msgDispatched(bool value = true) {}
  Mutex& getUserMutex() {}
  bool isAtExit() {}
};

TEST(PollFdMaintenanceTest, eventFds)
{
  EventFds eventFds;
}
#endif
