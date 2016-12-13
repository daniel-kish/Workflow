#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iterator>
#include <queue>
using namespace std;

struct Event {
	int A;
	int B;
};

ostream& operator<< (ostream& os, Event const& e)
{
	return os << e.A << ' ' << e.B << '\n';
}

struct Job {
	int i;
	int j;
	int D;
};

bool operator==(Job const& l, Job const& r)
{
	return l.i == r.i && l.j == r.j && l.D == r.D;
}

ostream& operator<< (ostream& os, Job const& job)
{
	return os << job.i << " - " << job.D << " - " << job.j;
}

template <class T>
ostream& operator<< (ostream& os, vector<T> const& jobs)
{
	for (auto const& j : jobs)
		os << j << '\n';
	return os;
}

vector<vector<Job>::const_iterator> find_all_in(vector<Job> const& jobs, int j)
{
	vector<vector<Job>::const_iterator> fnd;
	auto b = jobs.cbegin();
	while (true) {
		auto p = find_if(b, cend(jobs), [j](Job const& job) {
			return job.j == j;
		});
		if (p == cend(jobs))
			break;
		fnd.push_back(p);
		b = p + 1;
	}
	return fnd;
}

vector<vector<Job>::const_iterator> find_all_out(vector<Job> const& jobs, int i)
{
	vector<vector<Job>::const_iterator> fnd;
	auto b = jobs.cbegin();
	while (true) {
		auto p = find_if(b, cend(jobs), [i](Job const& job) {
			return job.i == i;
		});
		if (p == cend(jobs))
			break;
		fnd.push_back(p);
		b = p + 1;
	}
	return fnd;
}

int btA(vector<Job> const& jobs, int j, vector<Event> & events)
{
	if (events[j - 1].A != -1)
		return events[j - 1].A;

	auto ins = find_all_in(jobs, j);

	if (ins.empty()) // reached the start
		return (events[j - 1].A = 0);

	vector<int> As;
	for (auto& p : ins) {
		int Ap = btA(jobs, p->i, events);
		As.push_back(Ap + p->D);
	}
	events[j - 1].A = *max_element(begin(As), end(As));
	return events[j - 1].A;
}

int btB(vector<Job> const& jobs, int i, vector<Event> & events)
{
	auto outs = find_all_out(jobs, i);
	if (outs.empty()) // reached the end
		return events[i - 1].B;

	vector<int> Bs;
	for (auto& p : outs) {
		int Bp = btB(jobs, p->j, events);
		Bs.push_back(Bp - p->D);
	}
	events[i - 1].B = *min_element(begin(Bs), end(Bs));
	return events[i - 1].B;
}

vector<Job> read_data(string filename)
{
	vector<Job> jobs;
	ifstream is{filename};

	if (!is) {
		perror("error open file");
		terminate();
	}

	int s, D;
	vector<int> f;

	while (!is.eof())
	{
		string str;
		char c;
		while ((c = is.get()) != '\n' && !is.eof())
			str.push_back(c);
		istringstream iss{str};
		double Dval;
		iss >> s >> Dval;
		copy(istream_iterator<int>{iss}, istream_iterator<int>{}, back_inserter(f));
		for (int fin : f)
			jobs.push_back({s,fin,int(Dval)});
		f.clear();
	}
	return jobs;
}

void print_graph_data(vector<Job> const& jobs, int n, string filename)
{
	vector<int> mat(n*n, 0);
	ofstream os{filename};
	if (!os) { perror(string{"error open 'adj_mat' file "s + filename}.data()); terminate(); }

	for (Job const& job : jobs)
		mat[n * (job.i - 1) + job.j - 1] = 1;

	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; j++)
			os << mat[n * i + j] << ' ';
		os << '\n';
	}
}

bool on_critical(Job const& job, vector<Event> const& events)
{
	int Ai{events[job.i - 1].A};
	int Aj{events[job.j - 1].A};
	int Bi{events[job.i - 1].B};
	int Bj{events[job.j - 1].B};

	return (Bi == Ai) && (Bj == Aj) && (Bj - Bi == Aj - Ai) && (Aj - Ai == job.D);
}

void print_crit_path_data(vector<Job> const& jobs, vector<Event> const& events, string filename)
{
	vector<Job> crit_jobs;

	copy_if(begin(jobs), end(jobs), back_inserter(crit_jobs), [&events](const Job& job) {
		return on_critical(job, events);
	});

	ofstream os{filename};
	if (!os) { perror("error open out file"); terminate(); }

	for (Job const& cr_job : crit_jobs)
		os << cr_job.i << ' ' << cr_job.j << '\n';
}

int def_last_event(vector<Job> const& jobs)
{
	auto p = max_element(begin(jobs), end(jobs), [](Job const& p, Job const& q) {
		return max(p.i, p.j) < max(q.i, q.j);
	});
	return max(p->i, p->j);
}

vector<int> find_all_start_nodes(vector<Job> const& jobs, int last_event)
{
	vector<int> startNodes;
	for (int i = 1; i <= last_event; ++i) {
		auto ins = find_all_in(jobs, i);
		if (ins.empty())
			startNodes.push_back(i);
	}
	return startNodes;
}

void print_events(vector<Event> const& events)
{
	for (int i = 0; i < events.size(); ++i)
		cout << i + 1 << ": " << setw(3) << events[i].A << ' ' << setw(3) << events[i].B << '\n';
}

void merge_nodes(vector<int> const& sns, vector<Job>& jobs)
{
	auto b = begin(sns);
	for (auto p = b+1, e = end(sns); p != e; ++p)
	{
		jobs.push_back(Job{*b,*p,0});
	}
}

int main()
{
	//vector<Job> jobs{{1,2,5}, {1,3,6}, {2,3,3}, {2,4,8}, {3,5,2}, {3,6,11}, {4,5,0}, {4,6,1}, {5,6,12}};

	auto jobs = read_data(R"(C:\Users\Daniel\Documents\Visual Studio 2015\Projects\WorkFlow\WorkFlow\data7.txt)");
	int last_event = def_last_event(jobs);
	cout << jobs << '\n'; cout << "last event: " << last_event << '\n';

	vector<Event> events(last_event, Event{-1,-1});

	auto startNodes = find_all_start_nodes(jobs, last_event);
	cout << startNodes << '\n';
	merge_nodes(startNodes, jobs);

	btA(jobs, last_event, events);
	events.back().B = events.back().A;
	btB(jobs, 1, events);


	print_events(events);

	sort(begin(jobs), end(jobs), [](Job const& j1, Job const& j2) {
		return j1.i < j2.i;
	});

	for (auto& job : jobs) {
		cout << '(' << setw(2) << job.i << '-' << setw(2) << job.j << ')' << '[' << setw(2)<< job.D << ']' << ' ';
		cout << " KMR: ";
		cout << '(' << setw(3) << events[job.i - 1].A << " - " << setw(3) << events[job.j - 1].A << ')' << "; ";
		cout << " KMP: ";
		cout << '(' << setw(3) << events[job.i - 1].B << " - " << setw(3) << events[job.j - 1].B << ')' << ";  ";
		cout << "reserve = " << setw(3) << events[job.j - 1].B - events[job.i - 1].A - job.D << ' ';
		if (on_critical(job, events))
			cout << "crit\n";
		else cout << '\n';
	}

	string save_path = R"(C:\Users\Daniel\Documents\Visual Studio 2015\Projects\WorkFlow\WorkFlow\)";
	print_graph_data (jobs, last_event, save_path + "adj_mat.dat");
	print_crit_path_data (jobs, events, save_path + "cr_path.dat");
}