// copied from https://developer.gnome.org/gtk3/stable/gtk-getting-started.html
// https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
// sudo apt-get install libgtk-3-dev
// description this gtk gui for ubuntu should provide a interface to the VENICE
// 8489 PHY CHIP from microchip

#include <errno.h> // Error integer and strerror() function
#include <fcntl.h> // Contains file controls like O_RDWR
#include <gtk/gtk.h>
#include <string.h>
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

static void print_hello(GtkWidget *widget, gpointer data) {
  g_print("Hello World\n");
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  // create buttons
  GtkWidget *button, *button2;
  GtkWidget *button_box, *button_box2;

  // window
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
  //
  button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(window), button_box);
  //
  button_box2 = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(window), button_box2);
  int test = 2;
  char a[] = "hellox world";
  char b[] = "LINK UP";
  button = gtk_button_new_with_label(a);
  button2 = gtk_button_new_with_label(b);
// onclick1
  g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
  g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy),
                           window);
  gtk_container_add(GTK_CONTAINER(button_box), button);
// onclick2
   g_signal_connect(button2, "clicked", G_CALLBACK(print_hello), NULL);
  g_signal_connect_swapped(button2, "clicked", G_CALLBACK(gtk_widget_destroy),
                           window);
  gtk_container_add(GTK_CONTAINER(button_box2), button2);
//SHOW
  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  // create new app!
  GtkApplication *app;
  int status;
  printf("test");
  //###############################
  // serial port
  // ### here put in device names
  // maybe even to
  // but better put in a mask for input
  // make sure other devices are not already openeing this ports
  int serial_port = open("/dev/ttyACM1", O_RDWR); // e.g. arduino
  // Check for errors
  if (serial_port < 0) {
    printf("Error %i from open: %s\n", errno, strerror(errno));
  }
  // Create new termios struct, we call it 'tty' for convention
  // No need for "= {0}" at the end as we'll immediately write the existing
  // config to this struct
  struct termios tty;
  // Read in existing settings, and handle any error
  // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
  // must have been initialized with a call to tcgetattr() overwise behaviour
  // is undefined
  if (tcgetattr(serial_port, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
  }
  struct termios {
    tcflag_t c_iflag; /* input mode flags */
    tcflag_t c_oflag; /* output mode flags */
    tcflag_t c_cflag; /* control mode flags */
    tcflag_t c_lflag; /* local mode flags */
    cc_t c_line;      /* line discipline */
    cc_t c_cc[NCCS];  /* control characters */
  };

  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  // tty.c_cflag |= PARENB;  // Set parity bit, enabling parity
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in
                          // communication (most common)

  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &=
      ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)

  tty.c_cflag |=
      CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;

  tty.c_lflag &= ~ECHO;   // Disable echo
  tty.c_lflag &= ~ECHOE;  // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG;   // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                   ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g.
                         // newline chars)
  tty.c_oflag &=
      ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon
                        // as any data is received.
  tty.c_cc[VMIN] = 0;
  // Set in/out baud rate to be 9600
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);

  char read_buf[256];
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    // return 1;
  }

  // Write to serial port
  unsigned char msg[] = {'H', 'A', 'l', 'l', 'o', '\r'};
  for (int i = 0; i < 200; i++) {
    write(serial_port, msg, sizeof(msg));
  }
  // Read bytes. The behaviour of read() (e.g. does it block?,
  // how long does it block for?) depends on the configuration
  // settings above, specifically VMIN and VTIME
  // int n = read(serial_port, &read_buf, sizeof(read_buf));
  //   if (n < 0) {
  //       printf("Error reading: %s", strerror(errno));
  //       //return 1;
  //   }
  //     printf("Read %i bytes. Received message: %s", n, read_buf);

  close(serial_port);

  //--------#####################-  GUI

  app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv); // start it..
  g_object_unref(app);

  return status;
}
