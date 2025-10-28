#ifndef CGIEXECUTOR_HPP_
#define CGIEXECUTOR_HPP_

#include <vector>
#include <string>

// RFC 3875
// https://tex2e.github.io/rfc-translater/html/rfc3875.html
//
// # 擬似コード
// if http method == GET
//   dup new fs -> stdout
//   req = http リクエストを受け取る
//   script, envs = parse req
//   if script[0:1] == "#!" # shebang があるか確認
//     head_line = getline(script)
//     if is_executable(head_line[2:]) # shebang のパスは実行可能か？
//       exec(head_line, script, envs) # shebang のプログラムで実行
//     else throw error
//   else
//     exec(script, envs) # shebang がなかったらバイナリとして実行
//   return read_all fs
// else if http method == POST
// ...

class CgiExecutor {
 private:
	std::vector<std::string> envs;
 public:
 CgiExecutor();
 ~CgiExecutor();

 char **const getEnvs() const;
 void appendEnv(std::string &) const;
};

#endif
