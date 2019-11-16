add_library (Observers iFilter.h cBiquad.cpp cBiquad.h Observers.cpp Observers.h cStateObserver.cpp cStateObserver.h  )
set_property(TARGET Observers PROPERTY CXX_STANDARD 17)
