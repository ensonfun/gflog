gflog
=====

Seperate from Chromium project.A log writter for windows c++ projects.

中文说明：
这是一个 c++ 的程序日志记录库，从 Chromium 里面扒出来的，相比于 Google 开源的 glog, 这个可以自己完全定义输出文件名字，并且可以把单个程序的所有日志记录到一个文件中。同时，它还支持多线程以及 unicode. 
和 glog 类似，也支持大量的日志帮助宏。具体参考实例工程或者 glog 说明。

待添加功能：
当日志文件大小过大时，自动清理。
