#ifndef WINDOW_H
#define WINDOW_H

struct window {
    arena*                              Arena;
    array<gdi_handle<gdi_texture_view>> SwapchainViews;
    ui                                  UI;
    panel* RootPanel;
    panel* FreePanels;
};

struct window_storage {
    ak_async_slot_map64 SlotMap;
    window*             Windows;
};

void Window_Storage_Init(window_storage* Storage, arena* Arena, u32 Capacity);


#endif