Display driver:
    Functions: display rows of text, diff colors, (rounded) rectangles, etc.
Display singleton
    has methods that simply consume data and update physical state.
Menu system: The only thing that talks to the display. Has an OOP heirarchy of MenuNodes with children. Auto building scrollable lists. Talks to inputs. 
MenuNode:
  has name: string, children: \[MenuNode\]?, callback: callable?
  Later a bitmap icon or something
MenuManager:
  consumes keyboard inputs (only such system), renders the list of nodes, handles tree heirarchy.

Define modules for hardware-specific functions. 
    BLE and such start having troubles. If we scan for a list of devices, must we dynamically update global state?
Leaf nodes call arbitrary functions as their Callable