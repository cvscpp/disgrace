ASSERT INFO:
./src/common/strconv.cpp(1165): assert ""Assert failure"" failed in FromWChar(): trying to encode undefined Unicode character

BACKTRACE:
[1] _end
[2] _end
[3] _end
[4] _end
[5] _end
[6] _end
[7] _end
[8] _end
[9] _end
[10] wxString::ConvertedBuffer<char>::~ConvertedBuffer() /usr/local/include/wx-3.2/wx/string.h:3703
[11] disgrace_ns::DisgraceApp::OnInit() /home/mirsha/msdev/disgrace/src/main.cpp:81
[12] _end
[13] main /home/mirsha/msdev/disgrace/src/main.cpp:107
[14] _end
