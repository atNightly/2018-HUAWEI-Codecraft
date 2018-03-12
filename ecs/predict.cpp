#include "predict.h"

enum {
	CPU, MEM
} target;

std::pair<datetime, datetime> predict_interval;
char result[20 * 20 * 10000];

void interval_predict(std::map<string, int>& solution_flavor) {
	int during_days = predict_interval.second.date - predict_interval.first.date;
	for(const auto &f: flavors_info)
		solution_flavor[f.first] = get_interval_flavors_count(f.first, predict_interval.first.date + (-during_days), during_days);
}


void deploy_server(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server) {
	std::vector<server> servers;
	solution_server.emplace_back();
	servers.emplace_back();
	for(const auto & sf: solution_flavor) {
		string vm_name = sf.first;
		int vm_count = sf.second;
		const flavor_info& flv = flavors_info[vm_name];
		for (int f_count = 0; f_count < vm_count; ++f_count) {
			for(size_t i = 0; i < servers.size(); ++i) { // first fit
				if (flv <= servers[i]) {
					servers[i] -= flv;
					if(solution_server[i].find(vm_name) != solution_server[i].end())
						++solution_server[i][vm_name];
					else
						solution_server[i][vm_name] = 1;
					break;
				} else if(i == servers.size() - 1) {
					solution_server.emplace_back();
					servers.emplace_back();
				}
			}
		}
	}
}

char* get_result(std::map<string, int>& solution_flavor, std::vector<std::map<string, int>>& solution_server) {
	char buffer[20 * 20];
	char *pr = result, *pb = buffer;
	int flavor_count = 0;
	for(const auto & flv: solution_flavor)
		flavor_count += flv.second;

	snprintf(buffer, sizeof(buffer), "%d\n", flavor_count);

	pb = buffer; while(*pb && (*pr++ = *pb++));
	for(const auto & flv: solution_flavor) {
		snprintf(buffer, sizeof(buffer), "%s %d\n", flv.first.c_str(), flv.second);
		pb = buffer; while(*pb && (*pr++ = *pb++));
	}

	snprintf(buffer, sizeof(buffer), "\n%ld\n", solution_server.size());
	pb = buffer; while(*pb && (*pr++ = *pb++));

	for(size_t i = 0; i < solution_server.size(); ++i) {
		snprintf(buffer, sizeof(buffer), "%ld", i + 1);
		pb = buffer; while(*pb && (*pr++ = *pb++));
		for(const auto & flv: solution_server[i]) {
			snprintf(buffer, sizeof(buffer), " %s %d", flv.first.c_str(), flv.second);
			pb = buffer; while(*pb && (*pr++ = *pb++));
		}
		*pr++ = '\n';
	}

	return result;
}


//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	int line = 0;
	std::map<string, int> solution_flavor; // vm_name: count
	std::vector<std::map<string, int>> solution_server;

	sscanf(info[line], "%d %d %d", &server::cpu_count, &server::mem_size, &server::disk_size);
	server::mem_size *= 1024; // covert to MB
	line = read_flavors_info(info);

	info[line][3] = 0;
	target = strcmp(info[line], "CPU") == 0 ? CPU:MEM;
	line += 2;

	predict_interval.first = datetime(info[line++]);
	predict_interval.second = datetime(info[line]);

	read_flavors(data, data_num);
	interval_predict(solution_flavor);
	deploy_server(solution_flavor, solution_server);

	// 需要输出的内容

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(get_result(solution_flavor, solution_server), filename);
}
