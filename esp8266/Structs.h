#pragma once

typedef struct access_point_def {
  byte flags;
  uint32_t last_seen;
  byte *BSSID;
  char *SSID;

  bool getHidden() {
    return (flags & 0x80) != 0;
  }

  void setHidden(bool hidden) {
    if (hidden)
      flags |= 0x80;
    else
      flags &= ~0x80;
  }

  bool getSelected() {
    return (flags & 0x40) != 0;
  }

  void setSelected(bool selected) {
    if (selected)
      flags |= 0x40;
    else
      flags &= ~0x40;
  }

  bool getLeakingHidden() {
    return (flags & 0x20) != 0;
  }

  void setLeakingHidden(bool leaking) {
    if (leaking)
      flags |= 0x20;
    else
      flags &= ~0x20;
  }

  byte getChannel() {
    return flags & 0x0F;
  }

  void setChannel(byte channel) {
    flags = (flags & 0xF0) | (channel & 0x0F);
  }
} access_point;

typedef struct ap_node_def {
  access_point *ap;
  void *next;
} ap_node;

typedef struct ap_list_def {
  ap_node *head;
  ap_node *tail;
} ap_list;
