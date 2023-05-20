#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include <libinput.h>
#include <libudev.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

bool is_alt_pressed = false;

void simulate_key_alt(bool pressed) {
  Display *display = XOpenDisplay(NULL);
  if (!display) {
    printf("Failed to open display\n");
    return;
  }
  XTestFakeKeyEvent(display, XKeysymToKeycode(display, XK_Alt_L), pressed, CurrentTime);
  XFlush(display);
  XCloseDisplay(display);

}


void handle_event(struct libinput_event *event)
{
  switch (libinput_event_get_type(event)) {
    case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
      printf("hold begin %d\n", libinput_event_get_type(event));
      simulate_key_alt(true);
      break;
    case LIBINPUT_EVENT_GESTURE_HOLD_END:
      printf("hold end %d\n", libinput_event_get_type(event));
      simulate_key_alt(false);
    default:
      printf("other event %d\n", libinput_event_get_type(event));
      break;
  }
}

static int open_restricted(const char* path, int flags, void* user_data)
{
  int fd = open(path, flags);
  return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void* user_data)
{
  close(fd);
}

struct libinput_interface lit = {
  .open_restricted = open_restricted,
  .close_restricted = close_restricted,
};

struct udev *udev;
struct libinput *li;


int main()
{
  udev = udev_new();
  if (!udev) {
    printf("Failed to create udev context\n");
    return -1;
  }

  li = libinput_udev_create_context(&lit, NULL, udev);
  if (!li) {
    printf("Failed to create libinput context\n");
    udev_unref(udev);
    return -1;
  }

  if (libinput_udev_assign_seat(li, "seat0") != 0) {
    printf("Failed to assign seat\n");
    libinput_unref(li);
    udev_unref(udev);
    return -1;
  }

  struct pollfd fds = {
    .fd = libinput_get_fd(li),
    .events = POLLIN,
    .revents = 0,
  };

  // get event then process
  while (poll(&fds, 1, -1) > -1) {

    libinput_dispatch(li);
    struct libinput_event *event = libinput_get_event(li);

    if (!event)
      continue;

    handle_event(event);
    libinput_event_destroy(event);
  }

  libinput_unref(li);
  udev_unref(udev);

  return 0;
}
