void Job::remove_child(Job* child) => IMPLEMENTAR!
void JobGraph::erase(Job* job) e Job::~Job()

-----------------------------------------------------------

Se um vp não encontra trabalho, ele pede para o Daemon 
roubar trabalho para ele e se coloca na lista de vps 
em espera. Se o Daemon encontra, o vp recebe o trabalho 
e segue a execução. Caso contrário, o vp aguarda em uma
variável de condição até surgir novos trabalhos

-----------------------------------------------------------

Implementar a condição que dará signal nos vps aguardando 
por trabalho.
	- Isto inclui: De onde este signal virá

-----------------------------------------------------------

Computação verde o meu bago, isso é problema do "Alan do 
futuro" quando ele estará no mestrado, pois na proposta a
preocupação é ter uma ambiente funcionando e algum bom
desempenho. (Só um desabafo)