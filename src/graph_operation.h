#ifndef GRAPH_OPERATION_H
#define GRAPH_OPERATION_H

class GraphOperation {

public:
	GraphOperation ();
	//virtual ~GraphOperation ();
	virtual void commit() = 0;
};

class CreateOperation : public GraphOperation {

public:
	CreateOperation ();
	//virtual ~CreateOperation ();
	virtual void commit();
};

class JoinOperation : public GraphOperation {

public:
	JoinOperation ();
	//virtual ~JoinOperation ();
	virtual void commit();
};

class EndOfThreadOperation : public GraphOperation {

public:
	EndOfThreadOperation ();
	//virtual ~EndOfThreadOperation ();
	virtual void commit();
};

class GetJobOperation : public GraphOperation {

public:
	GetJobOperation ();
	//virtual ~EndOfThreadOperation ();
	virtual void commit();
};

#endif