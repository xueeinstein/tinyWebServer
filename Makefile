TARGET = server
MAIN = main.cpp

$(TARGET): $(MAIN)
	$(CXX) -pthread -v -o $@ $^

update:
	rm $(TARGET);
	$(CXX) -pthread -v -o $(TARGET) $(MAIN)
