/*
 * CIS 371 Web Browser
 *
 * Ron Patrick
 * Summer 2016
 *
 * Assignment Web Browser, Part 3 submission
 *
 * Assignment Web Browser, Part 2
 *
 * The URL_history works currently, but it doesn't
 * quite work as one would expect.  All I did was
 * put URLs in the vector in the order they were
 * visited and didn't remove them ever.  So you
 * can go back and forward, but the page you get
 * might not be the page you were expecting.
 * Just go back a little further if you didn't find
 * it in the first place.
 *
 * I'll be sure to fix the URL_history logic in
 * future editions of this program.
 *
 * Here is the commmand line need to compile the
 * code in EOS:
 * g++ -std=c++0x -I/usr/include/gtk-2.0 -I/usr/include/libsoup-2.4
 * 	-I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0
 * 	-I/usr/lib64/gtk-2.0/include -I/usr/include/pango-1.0
 * 	-I/usr/include/pango-1.0/pango -I/usr/include/cairo
 * 	-I/usr/lib64/glib-2.0/include -I/usr/lib64/glib-2.0
 * 	-I/usr/lib64/gtk-2.0 -I/usr/include/glib-2.0 -O3
 * 	-Wall -c -fmessage-length=0 -MMD -MP -MF"CIS371BasicWebBrowser.d"
 * 	-MT"CIS371BasicWebBrowser.o" -o "CIS371BasicWebBrowser.o"
 * 	"CIS371BasicWebBrowser.cpp"
 *
 * 	(linker)
 * 	g++ `pkg-config gtk+-2.0 --cflags --libs` -o "CIS371BasicWebBrowser"
 * 	CIS371BasicWebBrowser.o
 *
 * 	Also need the two "Back" and "Forward" Icons which I have
 * 	included in my 'part 2' submission.
 * 	('leftArrow.png' and 'rightArrow.png')
 * 	These need to be in the same directory as the linux
 * 	executable for this program, otherwise, it won't run.
 *
 * 	(the header files I constructed are needed obviously as well)
 * 	Here is the command-line unformatted for printout
 * 	g++ -std=c++0x -I/usr/include/gtk-2.0 -I/usr/include/libsoup-2.4 -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/lib64/gtk-2.0/include -I/usr/include/pango-1.0 -I/usr/include/pango-1.0/pango -I/usr/include/cairo -I/usr/lib64/glib-2.0/include -I/usr/lib64/glib-2.0 -I/usr/lib64/gtk-2.0 -I/usr/include/glib-2.0 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"CIS371WebBrowser.d" -MT"CIS371WebBrowser.o" -o "CIS371WebBrowser.o" "CIS371WebBrowser.cpp"
 * 	g++ `pkg-config gtk+-2.0 --cflags --libs` -o "CIS371BasicWebBrowser" CIS371BasicWebBrowser.o
 */

#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <gtk/gtk.h>
#include <algorithm>
#include <cairo.h>

// activate one of these to test the code
#define DO_MYURL_TEST false//true
#define DO_WEBTRANSACTIONCLIENT_TEST false

#define FONTSIZE 15.0
#define MARGIN 10.0

using namespace std;

static GdkCursor *_normal_cursor = NULL;
static GdkCursor *_hand_cursor = NULL;
string homeURL;

static void AddressBarEnterPressed(GtkWidget *widget, gpointer data);
static gboolean link_click (GtkWidget *, GdkEventButton *, gpointer);
void DoWebPage(string address, bool skipHistory);
void DoWebPage(string address);
gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gint configure_event(GtkWidget *widget, GdkEventConfigure *event);
void go_home(GtkWidget *widget,  gpointer item);
void forward_back(GtkWidget *widget,  gpointer item);
static gboolean _set_normal(GtkWidget *w, GdkEventCrossing *e, gpointer data);
static gboolean _set_hand(GtkWidget *w, GdkEventCrossing *e, gpointer data);
GtkWidget *address;
GtkWidget *drawArea;
GtkWidget *window;
GtkWidget *parentVbox;
GtkWidget *vbox2;
GtkWidget *vbox3;
bool gotImage=false;
GdkPixbuf *singleImage;
GtkWidget *singleImageWidget;
unordered_map<string,GdkPixbuf*> imageCache;

static GdkPixmap *pixmap = NULL;
int drawWidth;
int drawHeight;
string screenText;
int line_height;
double panel_width;
GdkRectangle drawingArea;
string currentlyDisplayedURL;
string newURL;
vector<string> URL_history;
int history_pointer=0;

unordered_map<string,GdkRectangle> links;
unordered_map<string,GtkWidget*> linkEventBoxes;
unordered_map<GtkWidget*,string> linkCrossRef;//

//My custom string split routine I've been using in
//most of my C++ programs for a while now.
vector<string> split(string s, string any_of) {
	vector<string> seps;
	string temp = s;
	int foundat = temp.find(any_of);
	while (foundat != -1 && temp.length() > 0) {
		seps.push_back(temp.substr(0, foundat));
		temp = temp.substr(foundat + 1, temp.length());
		foundat = temp.find(any_of);
	}
	if (temp.length() > 0) {
		seps.push_back(temp);
	}
	return seps;
}

bool EndsWith(string in, string sub) {
	int inLen=in.length();
	int subLen=sub.length();
	if(inLen<subLen) {
		return false;
	}
	if(in.substr(inLen-subLen,subLen)==sub) {
		return true;
	}
	return false;
}

bool StartsWith(string in, string sub) {
	int inLen=in.length();
	int subLen=sub.length();
	if(inLen<subLen) {
		return false;
	}
	if(in.substr(0,subLen)==sub) {
		return true;
	}
	return false;
}

//I sort of mirrored the methods given in "StarterDisplay.java"
//so my program would function roughly the same as the assignment
//was intended to.
string Clip(string in) {
	if(in.length()<2) {
		return in;
	}
	return in.substr(1,in.length()-2);
}

#include "MyURL.h"
#include "MyURLTest.h"
#include "WebTransactionClient.h"
#include "WebTransactionClientTester.h"

MyURL url;

void hello(GtkWidget *widget, gpointer data) {
	g_print("button pressed...\n");
}

gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {

	g_print("delete event occurred\n");

	return (TRUE);
}

void destroy(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

//Main: constructs the window using the various containers
//	and widgets available from GTK and Cairo and presents
//	them.
int main(int argc, char *argv[]) {

	drawingArea=GdkRectangle();
	screenText="*CIS 371\nBasic Web Browser\nSummer 2016\nRon Patrick*\n";
	screenText+="Try starting here: *_[[http://www.cis.gvsu.edu/~kurmasz/";
	screenText+="Teaching/Courses/S16/CS371/Assignments/WebBrowser/";
	screenText+="sampleInput/basic.txt]]_*\n";
	screenText+="or here: *_[[http://162.192.254.238:8080/basic.txt]]_*\n";
	screenText+="or here: *_[[http://localhost:8080/basic.txt]]_*\n";
	screenText+="or here: *_[[http://162.192.254.238:8080/MyMarkupTest.txt]]_*\n";

	homeURL=screenText;

	if (DO_MYURL_TEST) {
		MyURLTest();
		return 0;
	}
	if (DO_WEBTRANSACTIONCLIENT_TEST) {
		WebTransactionClientTest();
		return 0;
	}

    GtkWidget *menu;
    GtkWidget *menu_bar;
    GtkWidget *root_menu;
    GtkWidget *menu_items;
    //GtkWidget *vbox;
    GtkWidget *button;
    GtkWidget *table, *label;

    int iconSize=40;
    int toolbarBorder=3;
    int menubarHeight=iconSize+(toolbarBorder*2);
    int menubarWidth=40;
    int addressBarHeight=25;
    int toolbarHeight=60;//menubarHeight;
    int y=0;

    gtk_init (&argc, &argv);

    // create a new window
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    //gtk_window_set_policy((GtkWindow *)window, FALSE, FALSE, FALSE);

	GdkScreen *currentScreen = gdk_screen_get_default();
	int screenHeight = gdk_screen_get_height(currentScreen);
	int screenWidth = gdk_screen_get_width(currentScreen);
	gtk_window_set_default_size(GTK_WINDOW(window),screenWidth-10,screenHeight-60);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_maximize(GTK_WINDOW(window));

    //gtk_widget_set_usize (GTK_WIDGET (window), 200, 100);
    gtk_window_set_title (GTK_WINDOW (window), "CIS371 Basic Web Browser");
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                        (GtkSignalFunc) gtk_main_quit, NULL);

    //GtkWidget *parentVbox = gtk_vbox_new(FALSE, 0);
    parentVbox = gtk_fixed_new();

    menu = gtk_menu_new ();
	menu_items = gtk_menu_item_new_with_label ("Quit");

	// ...and add it to the menu.
	gtk_menu_append (GTK_MENU (menu), menu_items);

	//gtk_signal_connect_object (GTK_OBJECT (menu_items), "activate",
	//	GTK_SIGNAL_FUNC (menuitem_response), (gpointer) g_strdup ("quit"));
	gtk_signal_connect_object (GTK_OBJECT (menu_items), "activate",
			(GtkSignalFunc) gtk_main_quit, (gpointer) g_strdup ("quit"));

    root_menu = gtk_menu_item_new_with_label ("File");

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (root_menu), menu);

    // Create a menu-bar to hold the menus and add it to our main window
    menu_bar = gtk_menu_bar_new ();
    //gtk_box_pack_start (GTK_BOX (vbox), menu_bar, FALSE, FALSE, 2);

    gtk_fixed_put(GTK_FIXED(parentVbox), menu_bar, 0, y);

    //gtk_widget_set_size_request(menu_bar, screenWidth-10, 30);
    gtk_widget_set_size_request(menu_bar, menubarWidth, menubarHeight);

    GdkPixbuf *iconLeftP, *iconRightP;
    GdkBitmap *mask;
    GtkToolItem *iconLeft, *iconRight;
	GtkWidget *toolbar, *iconLeftW, *iconRightW;
    GdkPixbufLoader *loader, *loader2;

    //Load icons for forward and backward toolbar buttons
    fstream inFile;

    inFile.open("leftArrow.png",ios::in|ios::binary|ios::ate);
    unsigned long fsize=inFile.tellg();
    inFile.seekg(0,ios::beg);
    unsigned char inImage[fsize];
    inFile.read((char*)inImage,fsize);
    inFile.close();
    loader = gdk_pixbuf_loader_new();
	GError *error = NULL;
	gdk_pixbuf_loader_write(loader, inImage, fsize, &error);
	if (error != NULL) {
		printf("%s\n", error[0].message);
		return 1;
	}
	iconLeftP = gdk_pixbuf_loader_get_pixbuf(loader);
	int iconPad=12;
	iconLeftP=gdk_pixbuf_scale_simple(iconLeftP,iconSize-iconPad,iconSize-iconPad,GDK_INTERP_BILINEAR);
	iconLeftW = gtk_image_new_from_pixbuf(iconLeftP);
	//image = gtk_image_new_from_file ("myfile.png");

	fstream inFile2;
    inFile2.open("rightArrow.png",ios::in|ios::binary|ios::ate);
    unsigned long fsize2=inFile2.tellg();
    inFile2.seekg(0,ios::beg);
    unsigned char inImage2[fsize2];
    inFile2.read((char*)inImage2,fsize2);
    inFile2.close();
    loader2 = gdk_pixbuf_loader_new();
	GError *error2 = NULL;
	gdk_pixbuf_loader_write(loader2, inImage2, fsize2, &error2);
	if (error2 != NULL) {
		printf("%s\n", error2[0].message);
		return 1;
	}
	iconRightP = gdk_pixbuf_loader_get_pixbuf(loader2);
	iconRightP = gdk_pixbuf_scale_simple(iconRightP,iconSize-iconPad,iconSize-iconPad,GDK_INTERP_BILINEAR);
	iconRightW = gtk_image_new_from_pixbuf(iconRightP);

    toolbar = gtk_toolbar_new();
    gtk_container_set_border_width (GTK_CONTAINER(toolbar), toolbarBorder);
    gtk_widget_set_size_request(toolbar,(iconSize+toolbarBorder+20)*4,iconSize+(toolbarBorder*2));
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
    iconLeft = gtk_tool_button_new(iconLeftW,"Back");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), iconLeft, 0);
    iconRight = gtk_tool_button_new(iconRightW,"Forward");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), iconRight, 1);

    GtkToolItem *homeTest=gtk_tool_button_new_from_stock(GTK_STOCK_HOME);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), homeTest,2);
    //GtkToolItem *prevTest=gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS);
    //gtk_toolbar_insert(GTK_TOOLBAR(toolbar), prevTest,2);
    //GtkToolItem *nextTest=gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
    //gtk_toolbar_insert(GTK_TOOLBAR(toolbar), nextTest,3);

    gtk_fixed_put(GTK_FIXED(parentVbox), toolbar, menubarWidth, y);
    y+=menubarHeight;

    g_signal_connect(G_OBJECT(iconLeft), "clicked",
            G_CALLBACK(forward_back), iconLeft);
    g_signal_connect(G_OBJECT(iconRight), "clicked",
            G_CALLBACK(forward_back), iconRight);

    g_signal_connect(G_OBJECT(homeTest), "clicked",
                G_CALLBACK(go_home), homeTest);

    gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), root_menu);

	table = gtk_table_new (1, 2, FALSE);

    gtk_fixed_put(GTK_FIXED(parentVbox), table, 0, y);
    gtk_widget_set_size_request(table, screenWidth-(MARGIN*2), addressBarHeight);

	label = gtk_label_new ("Address: ");
	gtk_widget_set_size_request(label, 80, 20);

	gtk_misc_set_alignment (GTK_MISC (label), 0, 0);

	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), (GtkAttachOptions)(GTK_FILL | GTK_SHRINK), 1, 1);

	//create a text box
	address = gtk_entry_new ();

	gtk_widget_set_size_request(address, (screenWidth-80)-(MARGIN*2), 20);
	y+=addressBarHeight;
	gtk_entry_set_max_length (GTK_ENTRY (address),0);

	gtk_table_attach(GTK_TABLE(table), address, 1, 2, 0, 1,
				(GtkAttachOptions)(GTK_FILL | GTK_SHRINK), (GtkAttachOptions)(GTK_FILL | GTK_SHRINK), 1, 1);
	g_signal_connect(address, "activate", G_CALLBACK(AddressBarEnterPressed), NULL);

	gtk_container_add (GTK_CONTAINER (window), parentVbox);
	gtk_widget_set_size_request(parentVbox, screenWidth-(MARGIN*2), screenHeight-y-(MARGIN*2));

	//GtkWidget *vbox2=gtk_vbox_new (FALSE, 0);
	vbox2 = gtk_scrolled_window_new (NULL,NULL);
	vbox3 = gtk_fixed_new();
	//gtk_fixed_put(GTK_FIXED(vbox3), vbox2, 0, y);
	//gtk_widget_set_usize(vbox2,100,100);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (vbox2),
	                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	drawArea = gtk_drawing_area_new();

	//gtk_container_add(GTK_CONTAINER(vbox2),drawArea);
	//gtk_scrolled_window_add_with_viewport (
	//                   GTK_SCROLLED_WINDOW (vbox2), drawArea);
	gtk_fixed_put(GTK_FIXED(vbox3), drawArea, 0, 0);
	gtk_scrolled_window_add_with_viewport (
		                   GTK_SCROLLED_WINDOW (vbox2), vbox3);
	gtk_widget_show(vbox3);
	//gtk_box_pack_start (GTK_BOX (vbox2), drawArea, TRUE, TRUE, 0);
	gtk_fixed_put(GTK_FIXED(parentVbox), vbox2, 0, y);
	y=15;
	drawingArea.y=y;
	drawingArea.x=MARGIN;
	drawingArea.width=screenWidth-(MARGIN*2);
	drawingArea.height=screenHeight-y-(MARGIN*2);
	drawWidth=screenWidth-(MARGIN*2);
	drawHeight=(screenHeight-90)-(MARGIN*2);
	gtk_widget_set_size_request(drawArea, drawWidth, drawHeight+1000);
	gtk_widget_set_size_request(vbox2, drawWidth, drawHeight);
	gtk_widget_set_size_request(vbox3, drawWidth, drawHeight);

	//gtk_widget_set_size_request(parentVbox, screenWidth-6, screenHeight-y);

	g_signal_connect (G_OBJECT (drawArea), "expose-event",
	                    G_CALLBACK (expose_event_callback), NULL);
	gtk_signal_connect (GTK_OBJECT(drawArea),"configure_event",
	                      (GtkSignalFunc) configure_event, NULL);
	gtk_widget_set_events (drawArea, GDK_EXPOSURE_MASK
	                         | GDK_LEAVE_NOTIFY_MASK
	                         | GDK_BUTTON_PRESS_MASK
	                         | GDK_POINTER_MOTION_MASK
	                         | GDK_POINTER_MOTION_HINT_MASK);

	gtk_widget_show_all(window);

    _normal_cursor = gdk_window_get_cursor((GdkWindow*)window);
    _hand_cursor = gdk_cursor_new(GDK_HAND2);

    gtk_main ();

	return 0;
}

//This is used to catch keyboard enter presses when typing in
//an address to the address bar
static void AddressBarEnterPressed(GtkWidget *widget, gpointer data) {
	DoWebPage(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void DoWebPage(string nAddress, bool skipHistory) {

	gdk_window_set_cursor(window->window, _normal_cursor);
	newURL=nAddress;
	url = MyURL(nAddress,&url);
	string uString=url.toString();
	if(!skipHistory) {
		if(URL_history.size()>0) {
			if(URL_history.back()!=uString) {
				URL_history.push_back(uString);
				history_pointer=URL_history.size()-1;
			}
		}
		else {
			URL_history.push_back(uString);
			history_pointer=URL_history.size()-1;
		}
	}
	gtk_entry_set_text(GTK_ENTRY(address),uString.c_str());
	WebTransactionClient client = WebTransactionClient(url);
	if(client.responseCode()==404) {
		screenText="_*Web Page Not Found (404)*_";
		gdk_window_invalidate_rect(window->window,&drawingArea,TRUE);
		client.finalize();
		return;
	}
	string cType=client.getHeader("content-type");
	if(StartsWith(cType,"text")) {
		screenText=client.getText();
	}
	else {
		screenText="";
		if(StartsWith(cType,"image")) {
			singleImage=client.getImage();
			gotImage=true;
		}
		else {
			gotImage=false;
		}
	}
	client.finalize();
	gdk_window_invalidate_rect(window->window,&drawingArea,TRUE);
}

void DoWebPage(string nAddress) {
	DoWebPage(nAddress,false);
}

void go_home(GtkWidget *widget,  gpointer item) {
	screenText=homeURL;
	gdk_window_invalidate_rect(window->window,&drawingArea,TRUE);
}

//This handles the forward and back button on my toolbar
void forward_back(GtkWidget *widget,  gpointer item) {
	GtkToolButton *t=(GtkToolButton*)widget;
	string name=gtk_tool_button_get_label(t);
	if(name=="Back") {
		if(URL_history.size()>0) {
			history_pointer--;
			int size=URL_history.size();
			if(history_pointer>-1 && history_pointer<size) {
				DoWebPage(URL_history.at(history_pointer),true);
			}
			else {
				history_pointer++;
			}
		}
		return;
	}
	if(name=="Forward") {
		if(URL_history.size()>0) {
			history_pointer++;
			int size=URL_history.size();
			if(history_pointer>-1 && history_pointer<size) {
				DoWebPage(URL_history.at(history_pointer),true);
			}
			else {
				history_pointer--;
			}
		}
		return;
	}
}

//This is used to add invisible event boxes to cover the links
//so I know when the mouse is hovering over one of them.
//I had the program change the mouse cursor when hovering
//over a link.  Just like they do in actual browsers.
int maxX=0;
int maxY=0;
void AddEventBoxForLink(string tempLink) {
	linkEventBoxes[tempLink]=gtk_event_box_new();
	gtk_widget_set_size_request(linkEventBoxes[tempLink], links[tempLink].width, links[tempLink].height);
	gtk_fixed_put(GTK_FIXED(vbox3), linkEventBoxes[tempLink], links[tempLink].x, links[tempLink].y-line_height);
	int tempMaxX=links[tempLink].x+links[tempLink].width;
	int tempMaxY=(links[tempLink].y-line_height)+links[tempLink].height;
	if(tempMaxX>maxX){maxX=tempMaxX;}
	if(tempMaxY>maxY){maxY=tempMaxY;}
	//gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (vbox2), drawArea);
	//gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW (vbox2),)
	gtk_event_box_set_above_child((GtkEventBox*)linkEventBoxes[tempLink],true);
	gtk_event_box_set_visible_window((GtkEventBox*)linkEventBoxes[tempLink],false);
	gtk_widget_set_events(linkEventBoxes[tempLink], GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_show(linkEventBoxes[tempLink]);
	g_signal_connect (G_OBJECT (linkEventBoxes[tempLink]),
		"button_press_event",
		G_CALLBACK (link_click), linkEventBoxes[tempLink]);
	g_signal_connect_swapped(linkEventBoxes[tempLink], "enter_notify_event", G_CALLBACK(_set_hand), window);
	g_signal_connect_swapped(linkEventBoxes[tempLink], "leave_notify_event", G_CALLBACK(_set_normal), window);
	linkCrossRef.insert(pair<GtkWidget*,string>(linkEventBoxes[tempLink],tempLink));
}

//This method has all the drawing responsibilities for
//the drawing area below the address bar
static void draw (GdkDrawable *d, GdkGC *gc) {

	if(newURL!=currentlyDisplayedURL) {
		for(auto e:linkCrossRef) {
			gtk_widget_destroy(e.first);
		}
		for(auto e:linkEventBoxes) {
			gtk_widget_destroy(e.second);
		}
		linkCrossRef.clear();
		linkEventBoxes.clear();
		links.clear();
	}

	panel_width = drawingArea.width;

	cairo_t *cr;
	cairo_text_extents_t txtExtents;
	cr = gdk_cairo_create(drawArea->window);

	double addtoDraw=20.0;

	if (screenText=="") {
		if(gotImage) {
			int width = gdk_pixbuf_get_width(singleImage);
			int height = gdk_pixbuf_get_height(singleImage);
			if(width==-1 && height==-1) {
				//gdk_cairo_set_source_pixbuf(cr,singleImage,drawingArea.x,drawingArea.y);
				//cairo_paint(cr);
				//return;
				screenText="Bad picture file";
				gotImage=false;
			}
			else {
				double leftX=drawingArea.x+MARGIN;
				double x=leftX;
				double y=(double)drawingArea.y+addtoDraw;
				cairo_set_source_rgb(cr, 0,0,0);
				cairo_rectangle(cr, drawingArea.x, drawingArea.y, drawingArea.width, drawingArea.height);
				cairo_fill(cr);
				if(width>drawingArea.width || height>drawingArea.height) {
					singleImage=gdk_pixbuf_scale_simple(singleImage,drawingArea.width-10,drawingArea.height-10,GDK_INTERP_BILINEAR);
					width = gdk_pixbuf_get_width(singleImage);
					height = gdk_pixbuf_get_height(singleImage);
					//cairo_scale(cr, drawingArea.width-200, drawingArea.height-200);
				}
				x=drawingArea.x+((double)drawingArea.width-(double)width)/2.0;
				y=drawingArea.y+((double)drawingArea.height-(double)height)/2.0;
				gdk_cairo_set_source_pixbuf(cr,singleImage,x,y);
				cairo_paint(cr);
				return;
			}
		}
	}

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, drawingArea.x, drawingArea.y, drawingArea.width, drawingArea.height+2000);
	cairo_fill(cr);
	cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, FONTSIZE);

	while(screenText.at(0)==' ' && screenText.length()>1) {
		screenText=screenText.substr(1,screenText.length()-1);
	}
	if(screenText.back()!='\0' && screenText.back()!='\n') {
		screenText+='\0';
	}

	cairo_set_source_rgb(cr, 0, 0, 0);

    cairo_text_extents(cr,"_`",&txtExtents);
    line_height = txtExtents.height;

    double leftX=drawingArea.x+MARGIN;
	double x=leftX;
	double y=(double)drawingArea.y+addtoDraw;
	cairo_move_to(cr, x, y);
	string word="";
	string altLink="";
	bool boldOn=false;
	bool italicsOn=false;
	bool linkOn=false;
	bool waitForLinkOne=false;
	bool imageOn=false;
	bool waitForImageOne=false;
	double blue=0.0;
	_cairo_font_slant slant=CAIRO_FONT_SLANT_NORMAL;//CAIRO_FONT_SLANT_OBLIQUE
	_cairo_font_weight weight=CAIRO_FONT_WEIGHT_NORMAL;//CAIRO_FONT_WEIGHT_BOLD
	string tempLink="";
	int wordDrawnAtX;
	int wordDrawnAtY;
	string lastLink;
	string imageName;
	maxX=0;
	maxY=0;

	for(auto c:screenText) {
		switch(c) {
		case '\r':
			break;
		case '[':
			if(waitForLinkOne) {
				linkOn=true;
				blue=1.0;
				cairo_set_source_rgb(cr, 0, 0, blue);
			}
			else {
				waitForLinkOne=true;
			}
			break;
		case '<':
			if(waitForImageOne) {
				imageOn=true;
			}
			else {
				waitForImageOne=true;
			}
			break;
		case '>':
			if(imageOn) {
				imageOn=false;
			}
			else {
				waitForImageOne=false;
				blue=0.0;
				imageName=word;
				word="";
				MyURL imageURL=MyURL(imageName,&url);
				GdkPixbuf *temp;
				bool missingImage=false;
				if(imageCache.find(imageURL.toString())==imageCache.end()) {
					WebTransactionClient imageClient=WebTransactionClient(imageURL);
					if(imageClient.responseCode()==200) {
						temp=imageClient.getImage();
						int width=gdk_pixbuf_get_width(temp);
						int height=gdk_pixbuf_get_height(temp);
						if(width!=-1 && height!=-1) {
							imageCache[imageURL.toString()]=temp;
							//imageCache.insert(pair<string,GdkPixbuf*>(imageURL.toString(),temp));
						}
						else {
							missingImage=true;
						}
					}
					else {
						missingImage=true;
					}
					imageClient.finalize();
				}
				else {
					temp=imageCache[imageURL.toString()];
				}
				if(missingImage) {
					imageName="Can't load image: "+imageName;
					cairo_text_extents(cr,imageName.c_str(),&txtExtents);
					if(txtExtents.x_advance+x>panel_width) {
						x=leftX;
						y+=line_height+3.0;
						cairo_move_to(cr,x,y);
					}
					cairo_show_text(cr, imageName.c_str());
					x+=txtExtents.x_advance;
				}
				else {
					gdk_cairo_set_source_pixbuf(cr,temp,x,y);
					cairo_paint(cr);
					cairo_set_source_rgb(cr, 0, 0, 0);
					int width=gdk_pixbuf_get_width(temp);
					int height=gdk_pixbuf_get_height(temp);
					//cout << imageURL.toString() << endl;
					//cout << "image start x: " << x << endl;
					//cout << "image start y: " << y << endl;
					if(width==-1 || height==-1) {
						//cout << "Invalid image dimensions\n";
					}
					else {
						x = x + width+5.0;
						y = y + height-line_height+3.0;
						//cout << "image end x: " << x << endl;
						//cout << "image end y: " << y << endl;
					}
				}
				if(x>maxX){maxX=x;}
				if(y>maxY){maxY=y;}
			}
			break;
		case ']':
			if(linkOn) {
				linkOn=false;
			}
			else {
				waitForLinkOne=false;
				if(altLink!="") {
					altLink=MyURL(altLink,&url).toString()+"\n"+word;
					links[altLink]=GdkRectangle();
					cairo_text_extents(cr,word.c_str(),&txtExtents);
					if(txtExtents.x_advance+x>panel_width) {
						links[altLink].x=leftX;
						links[altLink].y=y+line_height+3.0;
						links[altLink].width=leftX+txtExtents.x_advance;
						links[altLink].height=line_height+3.0;
					}
					else {
						links[altLink].x=x-txtExtents.x_advance;
						links[altLink].y=y;
						links[altLink].width=txtExtents.x_advance;
						links[altLink].height=line_height+3.0;
					}
					lastLink=altLink;
					altLink="";
				}
				else {
					lastLink=MyURL(word,&url).toString();
				}
			}
			break;
		case '*':
			if(boldOn) {
				boldOn=false;
			}
			else {
				boldOn=true;
				weight=CAIRO_FONT_WEIGHT_BOLD;
				cairo_select_font_face(cr, "sans-serif", slant, weight);
			}
			break;
		case '_':
			if(italicsOn) {
				italicsOn=false;
			}
			else {
				italicsOn=true;
				slant=CAIRO_FONT_SLANT_OBLIQUE;
				cairo_select_font_face(cr, "sans-serif", slant, weight);
			}
			break;
		case ' ':
			if(!linkOn) {
				word+=' ';
				cairo_text_extents(cr,word.c_str(),&txtExtents);
				if(txtExtents.x_advance+x>panel_width) {
					x=leftX;
					y+=line_height+3.0;
					cairo_move_to(cr,x,y);
				}
				cairo_show_text(cr, word.c_str());
				if(blue==1.0) {
					word.pop_back();
					cairo_text_extents(cr,word.c_str(),&txtExtents);
					tempLink="";
					if(lastLink=="") {
						tempLink=MyURL(word,&url).toString();
					}
					else {
						tempLink=lastLink;
					}
					if(links.find(tempLink)==links.end()) {
						links[tempLink]=GdkRectangle();
					}
					links[tempLink].x=x;
					links[tempLink].y=y;
					links[tempLink].width=txtExtents.x_advance;
					links[tempLink].height=line_height+3.0;
					if(linkEventBoxes.find(tempLink)==linkEventBoxes.end()) {
						AddEventBoxForLink(tempLink);
					}
					gtk_widget_show(linkEventBoxes[tempLink]);
				}
				blue=0.0;
				if(!italicsOn) {
					slant=CAIRO_FONT_SLANT_NORMAL;
				}
				if(!boldOn) {
					weight=CAIRO_FONT_WEIGHT_NORMAL;
				}
				cairo_set_source_rgb(cr, 0, 0, blue);
				cairo_select_font_face(cr, "sans-serif", slant, weight);
				word="";
				x+=txtExtents.x_advance;
				if(x>maxX){maxX=x;}
				if(y>maxY){maxY=y;}
			}
			else {
				if(altLink=="") {
					altLink=MyURL(word,&url).toString();
					word="";
				}
				else {
					word+=c;
				}
			}
			break;
		case '\n': case '\0':
			cairo_text_extents(cr,word.c_str(),&txtExtents);
			wordDrawnAtX=x;
			wordDrawnAtY=y;
			if(txtExtents.x_advance+x>panel_width) {
				x=leftX;
				y+=line_height+3.0;
				wordDrawnAtX=x;
				wordDrawnAtY=y;
				cairo_move_to(cr,x,y);
				word+=' ';
				cairo_text_extents(cr,word.c_str(),&txtExtents);
				cairo_show_text(cr, word.c_str());
			}
			else {
				cairo_show_text(cr, word.c_str());
				x=leftX;
				y+=line_height+3.0;
				cairo_move_to(cr,x,y);
			}
			if(x>maxX){maxX=x;}
			if(y>maxY){maxY=y;}
			if(blue==1.0) {
				tempLink="";
				if(lastLink=="") {
					tempLink=MyURL(word,&url).toString();
				}
				else {
					tempLink=lastLink;
				}
				if(links.find(tempLink)==links.end()) {
					links[tempLink]=GdkRectangle();
				}
				links[tempLink].x=wordDrawnAtX;
				links[tempLink].y=wordDrawnAtY;
				links[tempLink].width=txtExtents.x_advance;
				links[tempLink].height=line_height+3.0;
				if(linkEventBoxes.find(tempLink)==linkEventBoxes.end()) {
					AddEventBoxForLink(tempLink);
				}
				gtk_widget_show(linkEventBoxes[tempLink]);
			}
			if(!linkOn) {
				blue=0.0;
			}
			if(!italicsOn) {
				slant=CAIRO_FONT_SLANT_NORMAL;
			}
			if(!boldOn) {
				weight=CAIRO_FONT_WEIGHT_NORMAL;
			}
			cairo_set_source_rgb(cr, 0, 0, blue);
			cairo_select_font_face(cr, "sans-serif", slant, weight);
			word="";
			break;
		default:
			word+=c;
		}
	}
	gtk_widget_set_size_request(vbox3, maxX, maxY);
	currentlyDisplayedURL=newURL;
}

//This is called by the GTK system whenever an area of the
//screen is invalidated.
gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, gpointer data) {

	GdkGC *gc;

	gc = gdk_gc_new (widget->window);
	draw (widget->window, gc);
	g_object_unref (gc);

	return TRUE;
}

//This is not used at the moment.  Only used for future reference
static gint configure_event(GtkWidget *widget, GdkEventConfigure *event) {
	if (pixmap) {
		gdk_pixmap_unref(pixmap);
	}
	pixmap = gdk_pixmap_new(widget->window,
						  widget->allocation.width,
						  widget->allocation.height,
						  -1);
	gdk_draw_rectangle (pixmap,
					  //widget->style->white_gc,
					  widget->style->black_gc,
					  TRUE,
					  0, 0,
					  widget->allocation.width,
					  widget->allocation.height);

	return TRUE;
}

static gboolean link_click (GtkWidget *event_box,
				GdkEventButton *event,
				gpointer        data) {
    //printf("Event box clicked at coordinates %f,%f\n", event->x, event->y);
    string tlink=linkCrossRef.find(event_box)->second;
    if (tlink.find("\n")!=string::npos) {
    	vector<string> tsplit=split(tlink,"\n");
    	tlink=tsplit.at(0);
    }
    //cout << "tlink: "+tlink+"\n";
    DoWebPage(tlink);
    return TRUE;
}

static gboolean _set_hand(GtkWidget *w, GdkEventCrossing *e, gpointer data) {
    gdk_window_set_cursor(w->window, _hand_cursor);
    return TRUE;
}

static gboolean _set_normal(GtkWidget *w, GdkEventCrossing *e, gpointer data) {
    gdk_window_set_cursor(w->window, _normal_cursor);
    return TRUE;
}
