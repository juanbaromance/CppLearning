#include "mutex_condition.hpp"
#include <iostream>



// newspaper principle
int main()
{
  const int q_depth = 15;
  mtq<int> back_pressure(q_depth);

  for (int i = 0; i < q_depth; ++i)
    back_pressure.push(1);

  block_q reader_q(q_depth);
  block_q writer_q(q_depth);

  std::string instring;
  std::istringstream is( instring = "abcdefghijklmnopqrstuvwxyz");
  thread_group tg;

  tg.push([&]() mutable { reader(reader_q, back_pressure, is); });

  latch l( std::thread::hardware_concurrency() );
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i)
    tg.push([&]() mutable { compressor(reader_q, writer_q, l); });

  tg.push([&]() mutable { writer_q_closer(writer_q, l); });

  std::ostringstream os;
  tg.push([&]() mutable { in_order_writer(writer_q, back_pressure, os); });
  tg.join_all();

  auditor( __PRETTY_FUNCTION__  + os.str() + std::string(" vs ") + instring );
}


void reader(block_q &reader_q, mtq<int> &back_pressure, std::istream &is) {

    int id = 0;

    auditor( __PRETTY_FUNCTION__ );
    while (is) {
        back_pressure.pop();
        reader_q.push({ id, read(is, 3)});
        auditor( __PRETTY_FUNCTION__  + std::string(".") + std::to_string(reader_q.size()) );
        ++id;
    }
    reader_q.set_done();
}
