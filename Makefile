TARGET = server
MAIN = main.cpp

$(TARGET): $(MAIN)
	$(CXX) -pthread  -o $@ $^

update:
	rm $(TARGET);
	#$(CXX) -pthread -v -o $(TARGET) $(MAIN)
	$(CXX) -pthread -o $(TARGET) $(MAIN)
