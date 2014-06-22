TARGET = cluster
SINGLE = node
CLUSTER= cluster.cpp
NODE = node.cpp

$(TARGET): $(CLUSTER)
	$(CXX) -pthread  -o $@ $^

$(SINGLE): $(NODE)
	$(CXX) -pthread  -o $@ $^

u:
	rm $(TARGET);
	#$(CXX) -pthread -v -o $(TARGET) $(MAIN)
	$(CXX) -pthread -o $(TARGET) $(CLUSTER)

s:
	$(CXX) -pthread -o $(SINGLE) $(NODE)