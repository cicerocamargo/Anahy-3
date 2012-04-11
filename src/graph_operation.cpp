#include "graph_operation.h"

GraphOperation::GraphOperation() {
	
}

CreateOperation::CreateOperation() {
	
}

void CreateOperation::commit() {
	// insert a new node in the graph
}

JoinOperation::JoinOperation() {
	
}

void JoinOperation::commit(){
	// fetch and (probably) remove a node from the graph
}


EndOfThreadOperation::EndOfThreadOperation() {
	
}

void EndOfThreadOperation::commit() {
	// change the state of a node in the graph 
}


GetJobOperation::GetJobOperation() {
	
}

void GetJobOperation::commit() {
	// allocate a node from the graph to a VP
}
