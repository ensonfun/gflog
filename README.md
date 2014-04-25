gflog
=====

Seperate from Chromium project.A log writter for windows c++ projects.

中文说明：
这是一个 c++ 的程序日志记录库，从 Chromium 里面扒出来的。

您可能会问，Google 不是有开源的 glog 吗？
glog 不支持 unicode.
glog 不支持完全自定义日志文件名称。即 glog 是按照 ThreadID ProcessID 等命名日志文件的。
glog 不支持将多个线程的日志输出到同一个文件。glog 是按照每个线程写一个日志文件的，经常我们有一个多线程的程序，为了能够看到上下文的流程关系，会让他们都输出在一个日志文件。

待添加功能：
当日志文件大小过大时，自动清理。
