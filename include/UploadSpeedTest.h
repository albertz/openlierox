/*
 *  UploadSpeedTest.h
 *  OpenLieroX
 *
 *  Created by Karel Petranek on 28.1.09
 *	code under LGPL
 *
 */

#ifndef __UPLOADSPEEDTEST_H__
#define __UPLOADSPEEDTEST_H__

#include "olx-types.h"
#include "HTTP.h"

#define UPLOAD_TEST_SERVERS "speedtestservers.txt"

// Upload speed test class
// Tests the internet upload speed
class UploadSpeedTest  {
public:
	UploadSpeedTest();
	UploadSpeedTest(const std::string& test_url);
	UploadSpeedTest(const UploadSpeedTest& oth) { operator =(oth); }
	UploadSpeedTest& operator =(const UploadSpeedTest& oth)  { assert(false); return *this; }

	~UploadSpeedTest();

public:
	struct TestData  {
		TestData(UploadSpeedTest *t, bool succ, float r) :
			test(t), succeeded(succ), rate(r) {}

		UploadSpeedTest *test;
		bool succeeded;
		float rate;
	};

	Event<TestData> onFinished;

private:
	std::string m_url;
	bool m_finished;
	CHttp m_http;
	AbsTime m_startTime;
	float m_rate;

private:
	void generateRandomData(size_t size, std::string& result);
	void Http_onFinished(CHttp::HttpEventData d);

public:
	void startTest();
	void cancelTest();

	bool hasFinished() const	{ return m_finished; }
	bool hasErrorOccured() const { return m_http.GetError().iError != HTTP_NO_ERROR; }
	HttpError getError() const	{ return m_http.GetError(); }
	float getUploadRate() const { return m_rate; }
	int getProgress() const;
};

#endif // __UPLOADSPEEDTEST_H__
