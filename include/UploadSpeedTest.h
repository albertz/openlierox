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

#include "HTTP.h"

#define UPLOAD_TEST_SERVERS "speedtestservers.txt"

// Upload speed test class
// Tests the internet upload speed
class UploadSpeedTest  {
public:
	UploadSpeedTest();
	UploadSpeedTest(const std::string& test_url);
	UploadSpeedTest(const UploadSpeedTest& oth) { operator =(oth); }

	~UploadSpeedTest();

public:
	struct TestData  {
		TestData(UploadSpeedTest *t, bool succ, float r) :
			test(t), succeeded(succ), rate(r) {}

		UploadSpeedTest *test;
		bool succeeded;
		float rate;
	};

private:
	std::string m_url;
	Event<TestData> *m_onFinished;
	Event<CHttp::HTTPEventData> m_httpFinishedEvent;
	bool m_finished;
	CHttp m_http;

private:
	void generateRandomData(size_t size, std::string& result);
	void onHttpFinished(CHttp::HTTPEventData d);

public:
	UploadSpeedTest& operator =(const UploadSpeedTest& oth)  {
		if (this != &oth) {
			m_url = oth.m_url;
			m_onFinished = oth.m_onFinished;
			m_httpFinishedEvent = oth.m_httpFinishedEvent;
			m_finished = oth.m_finished;
			m_http = oth.m_http;
		}
		return *this;
	}

public:
	void startTest();
	void cancelTest();

	bool hasFinished() const	{ return m_finished; }
	bool hasErrorOccured() const { return m_http.GetError().iError != HTTP_NO_ERROR; }
	HttpError getError() const	{ return m_http.GetError(); }
	float getUploadRate() const { return m_http.GetUploadSpeed(); }

	void setOnTestFinished(Event<TestData> *e)  { m_onFinished = e; }
};

#endif // __UPLOADSPEEDTEST_H__
