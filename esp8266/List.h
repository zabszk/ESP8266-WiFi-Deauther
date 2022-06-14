#pragma once
#include "Attack.h"
#include "UART.h"
#include "Structs.h"

class List {
  public:
    static void ap_list_init(ap_list *l);
    static void ap_list_append(ap_list *l, access_point *ap);
    static void ap_list_remove_index(ap_list *l, int index);
    static access_point *get_ap_by_index(ap_list *l, int index);
    static access_point *get_ap_by_bbsid(ap_list *l, byte *bbsid);
};
