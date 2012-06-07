#build *.pb.h and *.pb.cc
PROTOC_BIN = /Users/lee/Downloads/src/google_tools/darwin/bin/protoc

cpp_file: 
	$(PROTOC_BIN) protocol.proto --cpp_out=.
clean: 
	rm -f *~ *.pb.h *.pb.cc *.o