# Agent Notes (flopper-one firmware)

This file is for coding agents (and humans) to quickly understand the conventions in this repo.

## UI + input conventions

### Input dispatch model

- `InputDispatcher` (`include/input_dispatcher.h`) dispatches button events only to the **top** `InputHandler*` on its stack.
- `MenuManager` sits on the stack by default; when an `App` is entered, it is pushed on top.
- To exit an app, call `App::exit()` (it calls `on_exit()`, sets `wants_exit_`, and pops the input stack).

Implication: the component currently on top of the stack fully “owns” input and the display.

### List navigation

Avoid re-implementing UP/DOWN bounds checks.

- Use `flopper::ui::apply_list_nav(e, selected, count)` to mutate selection on `UP`/`DOWN`.
- When drawing a list with an index that may drift out of bounds (because items are dynamic), use `flopper::ui::clamp_index(selected, count)`.

Example pattern:

```cpp
void on_input(InputEvent e) override {
  if (e == InputEvent::LEFT) { exit(); return; }
  if (flopper::ui::apply_list_nav(e, selected_, items_count)) return;
  // handle CENTER/RIGHT/etc
}

void draw() override {
  flopper::ui::draw_status(Display::get_instance(), "My App");
  flopper::ui::draw_list(Display::get_instance(), items,
                         flopper::ui::clamp_index(selected_, items.size()));
}
```

### Standard UI building blocks

Prefer the shared helpers in `include/ui.h`:

- `draw_status(display, text)` for the header line.
- `draw_list(display, items, selected)` for the standard list.
- `draw_list_at(display, items, selected, y_start)` for list-like rendering that starts below some custom content.
- `draw_confirm(display, message)` for blocking confirmations.

## Where things live

- Entry point: `src/main.cpp` (`poll()` input, then `MenuManager::tick()`).
- Menu tree / app registration: `include/menu_tree.h`.
- Menu framework:
  - `include/menu_node.h`, `include/menu_manager.h`, `include/app.h`
- Apps: `include/apps/**` (header-only).

## Making changes safely

- Keep UI changes in `ui.h` when they’re generally useful; update call sites to reduce duplication.
- Don’t introduce dynamic allocation patterns in tight loops unless needed (embedded constraints).
- After changes, validate with PlatformIO: `pio run -e esp32dev`.
