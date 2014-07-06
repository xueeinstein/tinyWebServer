TARGET = cluster
SINGLE = node
CLUSTER= cluster.cpp
NODE = node.cpp

$(TARGET): $(CLUSTER)
	$(CXX) -pthread  -o $@ $^

$(SINGLE): $(NODE)
	$(CXX) -pthread  -o $@ $^
# used to update cluster 
u:
	rm $(TARGET);
#$(CXX) -pthread -v -o $(TARGET) $(MAIN)
	$(CXX) -pthread -o $(TARGET) $(CLUSTER)

#used to compile node
s:
	$(CXX) -pthread -o $(SINGLE) $(NODE)