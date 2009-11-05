/*
 *  UploadSpeedTest.cpp
 *  OpenLieroX
 *
 *  Created by Karel Petranek on 28.1.09
 *	code under LGPL
 *
 */

#include <time.h>
#include <stdlib.h>

#include "Options.h"
#include "UploadSpeedTest.h"
#include "FindFile.h"
#include "Debug.h"
#include "LieroX.h"


#define TEST_DATA_SIZE (300 * 1024) // In bytes

/////////////////////
// Constructor
UploadSpeedTest::UploadSpeedTest()
: m_finished(false)
{
	m_http.onFinished.handler() = getEventHandler(this, &UploadSpeedTest::Http_onFinished);

	// Read the URL from disk
	FILE *fp = OpenGameFile(std::string("cfg/") + UPLOAD_TEST_SERVERS, "rt");
	if (fp)  {
		m_url = ReadUntil(fp);
		fclose(fp);
	}
}

/////////////////////
// Constructor
UploadSpeedTest::UploadSpeedTest(const std::string &test_url)
: m_url(test_url), m_finished(false)
{
	m_http.onFinished.handler() = getEventHandler(this, &UploadSpeedTest::Http_onFinished);
}

////////////////////
// Destructor
UploadSpeedTest::~UploadSpeedTest()
{
	m_http.CancelProcessing();
	m_http.onFinished.handler() = null;
}

/////////////////////
// Generates random data for the test
void UploadSpeedTest::generateRandomData(size_t size, std::string& result)
{
	// Just fill the string with an uninitialized memory
	char *buf = new char[size];
	for( size_t i=0; i<size/2; i++ )
		((Uint16 *)buf)[i] = GetRandomInt(65536);
	result.append(buf, size);
	delete[] buf;
}

/////////////////////
// Starts the bandwidth test
void UploadSpeedTest::startTest()
{
	// Get some random data
	std::string random_data;
	generateRandomData(TEST_DATA_SIZE, random_data);
	m_finished = false;

	notes << "Starting an upload speed test to host " << m_url << endl;

	// Start uploading
	std::list<HTTPPostField> data;
	data.push_back(HTTPPostField(random_data, "binary/random", "data", ""));
	m_http.SendData(data, m_url, tLXOptions->sHttpProxy);
	m_startTime = tLX->currentTime;
}

/////////////////////
// Cancels the test
void UploadSpeedTest::cancelTest()
{
	m_finished = true;
	m_http.CancelProcessing();
}

/////////////////////
// Called by the HTTP class when the upload finishes
void UploadSpeedTest::Http_onFinished(CHttp::HttpEventData d)
{
	m_finished = true;
	TimeDiff testTime = tLX->currentTime - m_startTime;
	if( testTime.seconds() <= 0.01f )
		testTime = 0.01f;
	m_rate = (float)TEST_DATA_SIZE / testTime.seconds();
	hints << "UploadSpeedTest: testTime " << testTime.seconds() << " data size " << TEST_DATA_SIZE << " rate " << m_rate << endl;
	// Delegate the event
	onFinished.occurred(TestData(this, d.bSucceeded, m_rate));
}

////////////////////
// Returns progress of the test
int UploadSpeedTest::getProgress() const
{
	if (m_finished)
		return 100;

	if (m_http.GetDataToSendLength() == 0)
		return 100;

   return (int)(m_http.GetSentDataLen() * 100 / TEST_DATA_SIZE);
}

