velox
=====
velox is a simple window manager based on
[swc](http://github.com/michaelforney/swc). It is inspired by dwm and xmonad.

Design
------
velox uses tag-based window management similar to dwm. This allows you to
construct your workspace by selecting a set of tags to work with. However, in
contrast with dwm, windows do not have any screen associated with them; they are
shown on whichever screen has their tag selected, allowing you to easily move
windows between screens by selecting their tag on a different screen. This is
similar to the multi-monitor workspace switching in xmonad.

To ensure that we never attempt to show a window in two places at once, we have
to impose several constraints. First, each window must have exactly one tag. In
practice, I've found that I rarely intentionally mark a window with more than
one tag anyway. Second, when you select a tag that is currently displayed on a
different screen, the tag is first deselected from that screen.

Configuration
-------------
velox uses a text file for its configuration. The configuration file is
searched for first in `${HOME}/.velox.conf` and then `/etc/velox.conf`.

Internally, the configuration structure is organized as a tree. For example, the
identifier `window.border_width` refers to the property controlling the border
width of the windows, and the identifier `tag.3.toggle` refers to the action
that toggles visibility of the third tag.

The format is fairly simple. Each line consists of a command, followed by
command-specific arguments. The currently available commands are `set`,
`action`, `key`, `button` and `rule`.

### The `set` command
    set <property> <value>

The `set` command is used to set the value of a property. It takes as its first
argument the identifier of the property to set, and as its second argument the
new value. The interpretation of the strings provided as values is dependent
upon the property being set. For example, `window.border_color_active`
interprets its value as a hexadecimal color, while `tag.1.name` simply sets the
name of the tag to the given string.

### The `action` command
    action <identifier> <type> <type-specific-arguments>

The `action` command creates a new action that can be invoked using key
bindings. The first argument is an identifier to use for the action to be
created. The second argument is the type of action to be created. Right now, the
only action type is `spawn`. All remaining arguments are interpreted based on
the action type.

#### The `spawn` action type
    action <identifier> spawn <shell-command>

The `spawn` action type is used to create actions which spawn a command when
invoked. This is useful to create key bindings which launch a particular
program. Its arguments are treated as a single string and are interpreted by the
shell using `sh -c '<shell-command>'`.

### The `key` command
    key <keysym> <modifier-list> <action-when-pressed>[:<action-when-released>]

The `key` command registers a key binding which invokes a particular action. The
first argument is the name of the keysym to bind to. These can be found in
`/usr/include/xkbcommon/xkbcommon-keysyms.h` (dropping the `XKB_KEY_` prefix).
The second argument is a comma separated list of modifiers under which to
activate the binding. Valid modifiers are `ctrl`, `alt`, `logo`, `shift`, `any`,
or `mod`. `any` matches any set of modifiers, and `mod` is a convenience
modifier which refers to the value of the `mod` property. The third and final
argument are the identifier of the action(s) to invoke when the key is pressed
or released. The general form is `<action-when-pressed>:<action-when-released>`,
and either action identifier can be omitted if none is desired. For convenience,
the common case of `<action-when-pressed>:` can be written as just
`<action-when-pressed>`.

These actions can be internal to velox, such as `focus_next`, or
custom actions created with the `action` command.

### The `button` command
    button <button> <modifier-list> <action-when-pressed>[:<action-when-released>]

The `button` command works identically to `key`, except that instead of a
keysym, it takes a button name. Valid button names are `left`, `right`,
`middle`, `side`, and `extra`.

### The `rule` command
    rule <type> <identifier> action

The `rule` command creates a new rule that will match new windows when they are
created. This is useful to, for example, spawn certain windows on certain tags.
The first argument is one of `title` or `app_id` and indicates which property of
the window should be matched. The second argument is the value to match the
window property against. If the type is `title` it is compared with the window
title, and if the type is `app_id` it compared with the application ID (see the
`xdg_shell` protocol for more details). The last argument is the action to
execute when the newly created window matches the rule. This action is invoked
with the new window as an argument. An example to always spawn a window with
title `st` on tag 2, is:

    rule title st tag.2.apply

See velox.conf.sample for an example of a basic configuration file.

<!-- vim: set ft=markdown tw=80 spell : -->
