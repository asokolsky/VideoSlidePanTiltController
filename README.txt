This is Arduino code for autonomous video slider/panner/tilter.
No timelapse functionality supported.
Slide is provided by a DIY slider (IGUS hardware?) with DC motor.
Pan/Tilt is provided by a modified Bescor MP-101 head.

Controller - controls directly the motors, talks to remote, 
    has user interface - keypad and display.  Sources are here.

Doc - original as well as 3rd party documentation for the project and 
    its components

Libraries - original components reused by Controller and Remote.  
    3rd-party libraries used: 
      RadioHead - 
      LiquidCrystal - comes with Arduino IDE

Remote - offers user interface (display and keaypad) and talks to controller
    over the radio.

Read http://alexsokolsky.blogspot.com/p/video-slider-10.html for motivation.

Read doc/todo.txt for Version/release/issue history

Copyright © 2015 by Alex Sokolsky

This software is distributed under GPL terms, read COPYING.txt for details.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

