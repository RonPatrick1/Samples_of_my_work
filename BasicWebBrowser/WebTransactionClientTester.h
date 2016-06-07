/*
 * WebTransactionClientTester.h
 *
 *  Created on: May 16, 2016
 *      Author: Ron Patrick
 */

#ifndef SRC_WEBTRANSACTIONCLIENTTESTER_H_
#define SRC_WEBTRANSACTIONCLIENTTESTER_H_

class WebTransactionClientTest {

public:

	WebTransactionClientTest() {
		cout << "Running: fetchesHTML...\n";
		cout << doPassFail(fetchesHTML());
		cout << "Running: fetchesText...\n";
		cout << doPassFail(fetchesText());
		cout << "Running: fetchesJpeg...\n";
		cout << doPassFail(fetchesJpeg());
		cout << "Running: fetchesPng...\n";
		cout << doPassFail(fetchesPng());
		cout << "Running: fetchesGif...\n";
		cout << doPassFail(fetchesGif());
		cout << "Running: handles404...\n";
		cout << doPassFail(handles404());
		cout << "Running: handlesForward...\n";
		cout << doPassFail(handlesForward());
	}
	~WebTransactionClientTest() {
	}

	string doPassFail(bool in) {
		if (in) {
			return "Passed\n";
		}
		return "Failed\n";
	}
	bool fetchesHTML() {

		MyURL url = MyURL("cis.gvsu.edu/~kurmasz/Humor/stupid.html");
		WebTransactionClient client = WebTransactionClient(url);

		bool output = true;
		if (client.responseCode() != 200) {
			output = false;
			cout << "Response differs\n";
		}
		if (stoi(client.getHeader("content-length")) != 1780) {
			output = false;
			cout << "Reported length differs\n";
		}
		if (client.getHeader("content-type") != "text/html") {
			output = false;
			cout
					<< "Type differs: "
							+ client.getHeader("content-type") + "\n";
		}

		string payload = client.getText();
		if (payload.substr(0, 6) != "<HTML>") {
			output = false;
			cout
					<< "Starts incorrectly: " + payload.substr(0, 6)
							+ "\n";
		}
		if (payload.find("<LI>Surfing in Nebraska.\n")
				== string::npos) {
			output = false;
			cout << "Contents missing\n";
		}
		if (payload.substr(payload.length() - 8, 8) != "</HTML>\n") {
			output = false;
			cout << "Ends incorrectly\n";
		}
		return output;
		//Assert.assertTrue("Starts incorrectly", payload.startsWith("<HTML>"));
		//Assert.assertTrue("Contents missing", payload.contains("<LI>Surfing in Nebraska.\n"));
		//Assert.assertTrue("Ends incorrectly", payload.endsWith("</HTML>\n"));
	}

	bool fetchesText() {

		bool output = true;
		MyURL url =
				MyURL(
						"cis.gvsu.edu/~kurmasz/DistillerSetup/distillerSetup.txt");
		WebTransactionClient client = WebTransactionClient(url);

		if (stoi(client.getHeader("content-length")) != 2930) {
			output = false;
			cout
					<< "Reported length differs: "
							+ client.getHeader("content-length") + "\n";
		}
		if (client.getHeader("content-type") != "text/plain") {
			output = false;
			cout
					<< "Type differs: "
							+ client.getHeader("content-type") + "\n";
		}
		//Assert.assertEquals("Response differs", 200, client.responseCode());
		//Assert.assertEquals("Reported length differs", 2930, Integer.parseInt(client.getHeader("content-length")));
		//Assert.assertEquals("Type differs", "text/plain", client.getHeader("content-type"));

		string payload = client.getText();
		if (payload.substr(0, 12) != "The software") {
			output = false;
			cout
					<< "Starts incorrectly: " + payload.substr(0, 12)
							+ "\n";
		}
		if (payload.find("cd into $WS.\n\n") == string::npos) {
			output = false;
			cout << "Contents missing\n";
		}
		if (payload.substr(payload.length() - 22, 22)
				!= "17). Build Utilities.\n") {
			output = false;
			cout << "Ends incorrectly\n";
		}
		//Assert.assertTrue("Starts incorrectly", payload.startsWith("The software"));
		//Assert.assertTrue("Contents missing", payload.contains("cd into $WS.\n\n"));
		//Assert.assertTrue("Ends incorrectly", payload.endsWith("17). Build Utilities.\n"));
		return output;
	}

	bool fetchesJpeg() {

		bool output = true;
		// MyURL url = MyURL("cis.gvsu.edu/~kurmasz/buzz1.jpg");
		MyURL url =
				MyURL(
						"http://ww2.chemistry.gatech.edu/~lw26/structure/molecular_interactions/buzz.jpg");
		WebTransactionClient client = WebTransactionClient(url);

		if (stoi(client.getHeader("content-length")) != 255482) {
			output = false;
			cout
					<< "Reported length differs: "
							+ client.getHeader("content-length") + "\n";
		}
		if (client.getHeader("content-type") != "image/jpeg") {
			output = false;
			cout
					<< "Type differs: "
							+ client.getHeader("content-type") + "\n";
		}
		//Assert.assertEquals("Response differs", 200, client.responseCode());
		//Assert.assertEquals("Reported length differs", 255482, Integer.parseInt(client.getHeader("content-length")));
		//Assert.assertEquals("Type differs", "image/jpeg", client.getHeader("content-type"));

		GdkPixbuf *pixbuf;
		pixbuf = client.getImage();
		int width = gdk_pixbuf_get_width(pixbuf);
		int height = gdk_pixbuf_get_height(pixbuf);
		if (height != 822) {
			output = false;
			cout << "Incorrect height: " + to_string(height) + "\n";
		}
		if (width != 1000) {
			output = false;
			cout << "Incorrect width: " + to_string(width) + "\n";
		}

		//Image image = client.getImage();
		//Assert.assertNotNull("Problem loading image", image);
		//Assert.assertEquals("Incorrect height: ", 822, image.getHeight(null));
		//Assert.assertEquals("Incorrect width: ", 1000, image.getWidth(null));
		return output;
	}

	bool fetchesPng() {

		bool output = true;
		MyURL url =
				MyURL(
						"http://www.unixstickers.com/image/data/stickers/emacs/emacs.sh.png");
		WebTransactionClient client = WebTransactionClient(url);

		if (stoi(client.getHeader("content-length")) != 81938) {
			output = false;
			cout
					<< "Reported length differs: "
							+ client.getHeader("content-length") + "\n";
		}
		if (client.getHeader("content-type") != "image/png") {
			output = false;
			cout
					<< "Type differs: "
							+ client.getHeader("content-type") + "\n";
		}
		//Assert.assertEquals("Response differs", 200, client.responseCode());
		//Assert.assertEquals("Reported length differs", 81938, Integer.parseInt(client.getHeader("content-length")));
		//Assert.assertEquals("Type differs", "image/png", client.getHeader("content-type"));

		GdkPixbuf *pixbuf;
		pixbuf = client.getImage();
		int width = gdk_pixbuf_get_width(pixbuf);
		int height = gdk_pixbuf_get_height(pixbuf);
		if (height != 650) {
			output = false;
			cout << "Incorrect height: " + to_string(height) + "\n";
		}
		if (width != 650) {
			output = false;
			cout << "Incorrect width: " + to_string(width) + "\n";
		}
		//Image image = client.getImage();
		//Assert.assertNotNull("Problem loading image", image);
		//Assert.assertEquals("Incorrect height: ", 650, image.getHeight(null));
		//Assert.assertEquals("Incorrect width: ", 650, image.getWidth(null));
		return output;
	}

	bool fetchesGif() {

		bool output = true;
		MyURL url =
				MyURL(
						"http://cdn.osxdaily.com/wp-content/uploads/2013/07/apple-logo.gif");
		WebTransactionClient client = WebTransactionClient(url);

		if (stoi(client.getHeader("content-length")) != 9853) {
			output = false;
			cout
					<< "Reported length differs: "
							+ client.getHeader("content-length") + "\n";
		}
		if (client.getHeader("content-type") != "image/gif") {
			output = false;
			cout
					<< "Type differs: "
							+ client.getHeader("content-type") + "\n";
		}
		//Assert.assertEquals("Response differs", 200, client.responseCode());
		//Assert.assertEquals("Reported length differs", 9853, Integer.parseInt(client.getHeader("content-length")));
		//Assert.assertEquals("Type differs", "image/gif", client.getHeader("content-type"));

		GdkPixbuf *pixbuf;
		pixbuf = client.getImage();
		int width = gdk_pixbuf_get_width(pixbuf);
		int height = gdk_pixbuf_get_height(pixbuf);
		if (height != 761) {
			output = false;
			cout << "Incorrect height: " + to_string(height) + "\n";
		}
		if (width != 620) {
			output = false;
			cout << "Incorrect width: " + to_string(width) + "\n";
		}
		//Image image = client.getImage();
		//Assert.assertNotNull("Problem loading image", image);
		//Assert.assertEquals("Incorrect height: ", 761, image.getHeight(null));
		//Assert.assertEquals("Incorrect width: ", 620, image.getWidth(null));
		return output;
	}

	bool handles404() {
		MyURL url = MyURL("http://www.gatech.edu/noSuchFile.wxyz");
		WebTransactionClient client = WebTransactionClient(url);
		bool output = true;
		if (client.responseCode() != 404) {
			output = false;
			cout << "Response differs\n";
		}
		return output;
	}

	bool handlesForward() {
		MyURL url = MyURL("http://www.cis.gvsu.edu/~kurmasz");
		WebTransactionClient client = WebTransactionClient(url);

		bool output = true;
		if (client.responseCode() != 301) {
			output = false;
			cout << "Response differs\n";
		}
		if (client.getHeader("location")
				!= "http://www.cis.gvsu.edu/~kurmasz/") {
			output = false;
			cout
					<< "Location differs: "
							+ client.getHeader("location") + "\n";
		}
		return output;
		//Assert.assertEquals("Response differs: ", 301, client.responseCode());
		//Assert.assertEquals("Location differs: ", "http://www.cis.gvsu.edu/~kurmasz/", client.getHeader("location"));
	}
};

#endif /* SRC_WEBTRANSACTIONCLIENTTESTER_H_ */
