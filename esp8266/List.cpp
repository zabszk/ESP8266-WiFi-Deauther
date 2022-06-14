#pragma once

#include "List.h"

void List::ap_list_init(ap_list *l) {
  l->head = NULL;
  l->tail = NULL;
}

void List::ap_list_append(ap_list *l, access_point *ap) {
  if (l == NULL)
    return;

  ap_node *e = (ap_node*)malloc(sizeof(ap_node));
  e->ap = ap;
  e->next = NULL;

  if (l->tail == NULL) {
    l->head = e;
    l->tail = e;
    return;
  }

  l->tail->next = e;
  l->tail = e;
}

void List::ap_list_remove_index(ap_list *l, int index) {
  if (l == NULL || l->head == NULL)
    return;

  ap_node *p = NULL;
  ap_node *e = l->head;

  for (int i = 0; i < index; i++) {
    if (e == NULL)
      return;
    
    p = e;
    e = (ap_node*)e->next;
  }

  if (p == NULL) {
    l->head = (ap_node*)e->next;
    
    if (e->ap->SSID != NULL)
      free(e->ap->SSID);
    free(e->ap->BSSID);
    free(e->ap);
    free(e);
    return;
  }

  p->next = e->next;
  if (e->next == NULL)
    l->tail = p;

  if (e->ap->SSID != NULL)
    free(e->ap->SSID);
  free(e->ap->BSSID);
  free(e->ap);
  free(e);
}

access_point *List::get_ap_by_index(ap_list *l, int index) {
  if (l == NULL || l->head == NULL)
    return NULL;

  ap_node *e = l->head;

  for (int i = 0; i < index; i++) {
    if (e == NULL)
      return NULL;
    
    e = (ap_node*)e->next;
  }

  return e->ap;
}

access_point *List::get_ap_by_bbsid(ap_list *l, byte *bssid) {
  ap_node *e = l->head;
  while (e != NULL) {
    for (int i = 0; i < 6; i++)
      if (bssid[i] != (e->ap->BSSID)[i])
        goto noret;
        
    return e->ap;
    noret:
    e = (ap_node*)e->next;
  }

  return NULL;
}
