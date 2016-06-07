/*
 * MyURLTest.h
 *
 *  Created on: May 15, 2016
 *      Author: Ron Patrick
 */

#ifndef SRC_MYURLTEST_H_
#define SRC_MYURLTEST_H_

#include <string>
#include <iostream>
using namespace std;

class MyURLTest {

public:
	MyURLTest() {
		setupCurrent();
		cout
		<< "\nResult for tests all precede pass/fail and test name.\n\n";
		parsesURLWithAllComponents();
		cout << "running test: parsesURLWithAllComponents\n";
		parsesURLWithUnusualComponents();
		cout << "running test: parsesURLWithUnusualComponents\n";
		parsesURLWithAllComponentsExceptPort();
		cout << "running test: parsesURLWithAllComponentsExceptPort\n";
		parsesURLWithNoScheme();
		cout << "running test: parsesURLWithNoScheme\n";
		parsesURLWithNoSchemeAndNoPort();
		cout << "running test: parsesURLWithNoSchemeAndNoPort\n";
		parsesURLWithDomainOnly();
		cout << "running test: parsesURLWithDomainOnly\n";
		parsesURLWithDomainOnly_slash();
		cout << "running test: parsesURLWithDomainOnly_slash\n";
		parsesURLWithDomainAndPortOnly();
		cout << "running test: parsesURLWithDomainAndPortOnly\n";
		parsesURLWithDomainAndPortOnly_slash();
		cout << "running test: parsesURLWithDomainAndPortOnly_slash\n";
		parsesURLWithNoFile();
		cout << "running test: parsesURLWithNoFile\n";
		parsesURLWithNoExtension();
		cout << "running test: parsesURLWithNoExtension\n";
		parsesDomainOnly();
		cout << "running test: parsesDomainOnly\n";
		parsesDomainAndDirOnly();
		cout << "running test: parsesDomainAndDirOnly\n";
		parsesDomainPortAndDirOnly();
		cout << "running test: parsesDomainPortAndDirOnly\n";
		//(expected = NumberFormatException.class)
		complainsIfPortNotInteger();
		cout << "running test: complainsIfPortNotInteger\n";
		//(expected = RuntimeException.class)
		complainsIfStringEmpty();
		cout << "running test: complainsIfStringEmpty\n";
		//(expected = RuntimeException.class)
		complainsIfDomainMissing();
		cout << "running test: complainsIfDomainMissing\n";
		//(expected = RuntimeException.class)
		complainsIfPortOnly();
		cout << "running test: complainsIfPortOnly\n";
		//(expected = RuntimeException.class)
		complainsIfSchemeAndPortOnly();
		cout << "running test: complainsIfSchemeAndPortOnly\n";
		ignoresCurrentIfStringHasScheme();
		cout << "running test: ignoresCurrentIfStringHasScheme\n";
		ignoresCurrentIfStringHasSchemeButNoPort();
		cout
		<< "running test: ignoresCurrentIfStringHasSchemeButNoPort\n";
		ignoresCurrentIfStringHasSchemeButNoPath();
		cout
		<< "running test: ignoresCurrentIfStringHasSchemeButNoPath\n";
		appliesRelative_singleFile();
		cout << "running test: appliesRelative_singleFile\n";
		appliesRelative_subdir();
		cout << "running test: appliesRelative_subdir\n";
		appliesRelative_singleFileToDirectory();
		cout << "running test: appliesRelative_singleFileToDirectory\n";
		appliesRelative_subdirToDirectory();
		cout << "running test: appliesRelative_subdirToDirectory\n";
		appliesRelative_replacesFileWithNoExtension();
		cout
		<< "running test: appliesRelative_replacesFileWithNoExtension\n";
		appliesRelative_copiesScheme();
		cout << "running test: appliesRelative_copiesScheme\n";
		appliesRelative_copiesPort();
		cout << "running test: appliesRelative_copiesPort\n";
		fullURLtoString();
		cout << "running test: fullURLtoString\n";
		noPathToString();
		cout << "running test: noPathToString\n";
	}
	bool verifyParts(
			MyURL url,
			string scheme,
			string domain,
			int port,
			string path) {
		bool verified = true;
		if (scheme != url.scheme()) {
			cout
					<< "Scheme differs: " + scheme + ", " + url.scheme()
							+ "\n";
			verified = false;
		}
		if (domain != url.domainName()) {
			cout
					<< "Domain differs: " + domain + ", "
							+ url.domainName() + "\n";
			verified = false;
		}
		if (port != url.port()) {
			cout
					<< "Port differs: " + to_string(port) + ", "
							+ to_string(url.port()) + "\n";
			cout << "Path differs: " + path + ", " + url.path() + "\n";
			verified = false;
		}
		setupCurrent();
		return verified;
	}

	bool verifyParts(
			string urlString,
			string scheme,
			string domain,
			int port,
			string path) {
		return verifyParts(MyURL(urlString), scheme, domain, port, path);
	}

	bool verifyParts(
			string urlString,
			MyURL current,
			string scheme,
			string domain,
			int port,
			string path) {
		return verifyParts(MyURL(urlString, &current), scheme, domain,
				port, path);
	}

	string doPassFail(bool in) {
		if (in) {
			return "Passed ";
		}
		return "Failed ";
	}

	MyURL current1;

	void setupCurrent() {
		current1 = MyURL(
				"http://www.google.com/place1/place2/george.html");
	}

	////////////////////////////////////
	//
	// Test constructor MyURL(string)
	//
	////////////////////////////////////

	void parsesURLWithAllComponents() {
		cout
		<< doPassFail(
		verifyParts(
		"ftp://www.cis.gvsu.edu:73/dir1/dir2/file.html",
		"ftp", "www.cis.gvsu.edu", 73,
		"/dir1/dir2/file.html"));
	}

	void parsesURLWithUnusualComponents() {
		cout
		<< doPassFail(
		verifyParts(
		"unusual://whatis.this:2/dir1_a/dir2_b/small_file.html",
		"unusual", "whatis.this", 2,
		"/dir1_a/dir2_b/small_file.html"));
	}

	void parsesURLWithAllComponentsExceptPort() {
		cout
				<< doPassFail(
						verifyParts(
								"ftp://www.cis.gvsu.edu/dir1/dir2/file.html",
								"ftp", "www.cis.gvsu.edu", 80,
								"/dir1/dir2/file.html"));
	}

	void parsesURLWithNoScheme() {
		cout
				<< doPassFail(
						verifyParts(
								"cis.gvsu.edu:73/dir1/dir2/file.html",
								"http", "cis.gvsu.edu", 73,
								"/dir1/dir2/file.html"));
	}

	void parsesURLWithNoSchemeAndNoPort() {
		cout
				<< doPassFail(
						verifyParts(
								"www.cis.gvsu.edu/dir1/dir2/file.html",
								"http", "www.cis.gvsu.edu", 80,
								"/dir1/dir2/file.html"));
	}

	void parsesURLWithDomainOnly() {
		cout
				<< doPassFail(
						verifyParts("www.cis.gvsu.edu", "http",
								"www.cis.gvsu.edu", 80, "/"));
	}

	void parsesURLWithDomainOnly_slash() {
		cout
				<< doPassFail(
						verifyParts("www.cis.gvsu.edu/", "http",
								"www.cis.gvsu.edu", 80, "/"));
	}

	void parsesURLWithDomainAndPortOnly() {
		cout
				<< doPassFail(
						verifyParts("www.cis.gvsu.edu:73", "http",
								"www.cis.gvsu.edu", 73, "/"));
	}

	void parsesURLWithDomainAndPortOnly_slash() {
		cout
				<< doPassFail(
						verifyParts("gvsu.edu:73/", "http", "gvsu.edu",
								73, "/"));
	}

	void parsesURLWithNoFile() {
		cout
				<< doPassFail(
						verifyParts(
								"ssh://www.cis.gvsu.edu:133/dir1/dir2/",
								"ssh", "www.cis.gvsu.edu", 133,
								"/dir1/dir2/"));
	}

	void parsesURLWithNoExtension() {
		cout
				<< doPassFail(
						verifyParts(
								"ssh://cis.gvsu.edu:133/dir1/dir2/file",
								"ssh", "cis.gvsu.edu", 133,
								"/dir1/dir2/file"));
	}

	void parsesDomainOnly() {
		cout
				<< doPassFail(
						verifyParts("www.google.com", "http",
								"www.google.com", 80, "/"));
	}

	void parsesDomainAndDirOnly() {
		cout
				<< doPassFail(
						verifyParts("www.google.com/dir1/dir2", "http",
								"www.google.com", 80, "/dir1/dir2"));
	}

	void parsesDomainPortAndDirOnly() {
		cout
				<< doPassFail(
						verifyParts("www.google.com:66/dir1/dir2",
								"http", "www.google.com", 66,
								"/dir1/dir2"));
	}

	//(expected = NumberFormatException.class)
	void complainsIfPortNotInteger() {
		MyURL test = MyURL("www.google.com:fourteen/dir1/dir2");
		if (test.getLastErr() == test.BAD_PORT) {
			cout << "Passed ";
			return;
		}
		cout << "Failed ";
	}

	//(expected = RuntimeException.class)
	void complainsIfStringEmpty() {
		MyURL test = MyURL("");
		if (test.getLastErr()) {
			cout << "Passed ";
			return;
		}
		cout << "Failed ";
	}

	//(expected = RuntimeException.class)
	void complainsIfDomainMissing() {
		MyURL test = MyURL("http://");
		if (test.getLastErr()) {
			cout << "Passed ";
			return;
		}
		cout << "Failed ";
	}

	//(expected = RuntimeException.class)
	void complainsIfPortOnly() {
		MyURL test = MyURL(":98");
		if (test.getLastErr()) {
			cout << "Passed ";
			return;
		}
		cout << "Failed ";
	}

	//(expected = RuntimeException.class)
	void complainsIfSchemeAndPortOnly() {
		MyURL test = MyURL("http://:98");
		if (test.getLastErr()) {
			cout << "Passed ";
			return;
		}
		cout << "Failed ";
	}

	////////////////////////////////////////////
	//
	// Test constructor MyURL(string, MyURL)
	//
	/////////////////////////////////////////////

	void ignoresCurrentIfStringHasScheme() {
		cout
				<< doPassFail(
						verifyParts(
								"ftp://www.cis.gvsu.edu:73/dir1/dir2/file.html",
								current1, "ftp", "www.cis.gvsu.edu", 73,
								"/dir1/dir2/file.html"));
	}

	void ignoresCurrentIfStringHasSchemeButNoPort() {
		cout
				<< doPassFail(
						verifyParts(
								"ftp://www.cis.gvsu.edu/dir1/dir2/file.html",
								current1, "ftp", "www.cis.gvsu.edu", 80,
								"/dir1/dir2/file.html"));
	}

	void ignoresCurrentIfStringHasSchemeButNoPath() {
		cout
				<< doPassFail(
						verifyParts(
								"ssh://www.cis.gvsu.edu:133/dir1/dir2/",
								current1, "ssh", "www.cis.gvsu.edu",
								133, "/dir1/dir2/"));
	}

	void appliesRelative_singleFile() {
		MyURL url = MyURL("www.cis.gvsu.edu/dir1/dir2/dir3/file.html");
		cout
				<< doPassFail(
						verifyParts("newFile.xls", url, "http",
								"www.cis.gvsu.edu", 80,
								"/dir1/dir2/dir3/newFile.xls"));
	}

	void appliesRelative_subdir() {
		MyURL url = MyURL("www.cis.gvsu.edu/dir1/dir2/dir3/file.html");
		cout
				<< doPassFail(
						verifyParts("fred/barney/wilma.html", url,
								"http", "www.cis.gvsu.edu", 80,
								"/dir1/dir2/dir3/fred/barney/wilma.html"));
	}

	void appliesRelative_singleFileToDirectory() {
		MyURL url = MyURL("www.cis.gvsu.edu/dir1/dir2/dir3/");
		cout
				<< doPassFail(
						verifyParts("newFile.xls", url, "http",
								"www.cis.gvsu.edu", 80,
								"/dir1/dir2/dir3/newFile.xls"));
	}

	void appliesRelative_subdirToDirectory() {
		MyURL url = MyURL("www.cis.gvsu.edu/dir1/dir2/dir3/");
		cout
				<< doPassFail(
						verifyParts("fred/barney/wilma.html", url,
								"http", "www.cis.gvsu.edu", 80,
								"/dir1/dir2/dir3/fred/barney/wilma.html"));
	}

	void appliesRelative_replacesFileWithNoExtension() {
		MyURL url = MyURL("www.cis.gvsu.edu/dir1/dir2/dir3/file");
		cout
				<< doPassFail(
						verifyParts("newFile.xls", url, "http",
								"www.cis.gvsu.edu", 80,
								"/dir1/dir2/dir3/newFile.xls"));
	}

	void appliesRelative_copiesScheme() {
		MyURL url = MyURL(
				"ftp://www.cis.gvsu.edu/dir1/dir2/dir3/file.html");
		cout
				<< doPassFail(
						verifyParts("newFile.xls", url, "ftp",
								"www.cis.gvsu.edu", 80,
								"/dir1/dir2/dir3/newFile.xls"));
	}

	void appliesRelative_copiesPort() {
		MyURL url = MyURL(
				"ssh://www.cis.gvsu.edu:6676/dir1/dir2/dir3/file.html");
		cout
				<< doPassFail(
						verifyParts("newFile.xls", url, "ssh",
								"www.cis.gvsu.edu", 6676,
								"/dir1/dir2/dir3/newFile.xls"));
	}

	////////////////////////////////////////////
	//
	// Test toString
	//
	/////////////////////////////////////////////

	void fullURLtoString() {
		string urlString =
				"http://www.yahoo.com:8088/p1/d2/v3/mommy.html";
		MyURL url = MyURL(urlString);
		cout << urlString + ", " + url.toString() << endl;
		if (url.toString() == urlString) {
			cout << "Passed ";
		} else {
			cout << "Failed ";
		}
	}

	void noPathToString() {
		string urlString = "http://www.yahoo.com:8088";
		MyURL url = MyURL(urlString);
		// Note:  The '/' at the end isn't necessary.  Feel free to change this test if
		// you implemented toString differently.
		cout << urlString + ", " + url.toString() << endl;
		if (url.toString() == urlString) {
			cout << "Passed ";
		} else {
			cout << "Failed ";
		}
	}

	////////////////////////////////////////////
	//
	// Test equals (these tests are not needed)
	//
	/////////////////////////////////////////////

	void URLEqualToSelf() {
		MyURL url1 = MyURL("http://fred.com/blah/blah/blah.html");
		cout << url1.toString() + ", " + url1.toString() << endl;
	}

	void identicalURLsAreEqual() {
		MyURL url1 = MyURL("http://fred.com/blah/blah/blah.html");
		MyURL url2 = MyURL("http://fred.com/blah/blah/blah.html");
		cout << url1.toString() + ", " + url2.toString() << endl;
		cout << url2.toString() + ", " + url1.toString() << endl;
	}

	void differentSchemesNotEqual() {
		MyURL url1 = MyURL("http://fred.com/blah/blah/blah.html");
		MyURL url2 = MyURL("https://fred.com/blah/blah/blah.html");
		//Assert.assertNotEquals(url1, url2);
		//Assert.assertNotEquals(url2, url1);
	}

	void differentDomainssNotEqual() {
		MyURL url1 = MyURL("http://fred.com/blah/blah/blah.html");
		MyURL url2 = MyURL("http://ferd.com/blah/blah/blah.html");
		//Assert.assertNotEquals(url1, url2);
		//Assert.assertNotEquals(url2, url1);
	}

	void differentPortsNotEqual() {
		MyURL url1 = MyURL("http://fred.com:19/blah/blah/blah.html");
		MyURL url2 = MyURL("http://fred.com:22/blah/blah/blah.html");
		//Assert.assertNotEquals(url1, url2);
		//Assert.assertNotEquals(url2, url1);
	}

	void differentPathsNotEqual() {
		MyURL url1 = MyURL("http://fred.com/blah/blah/blah.html");
		MyURL url2 = MyURL("http://fred.com/blah/blah.html");
		//Assert.assertNotEquals(url1, url2);
		//Assert.assertNotEquals(url2, url1);
	}

	////////////////////////////////////////////
	//
	// Test hashCode (not needed)
	//
	/////////////////////////////////////////////

	void identicalURLsHaveSameHashCode() {
		MyURL url1 = MyURL("http://fred.com/blah/blah/blah.html");
		MyURL url2 = MyURL("http://fred.com/blah/blah/blah.html");
		//cout << url1.hashCode(), url2.hashCode());
	}
};

#endif /* SRC_MYURLTEST_H_ */
