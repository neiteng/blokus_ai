#include "dumpable.hpp"

void Dumpable::setDumper(std::shared_ptr<BlokusDumper> dumper_)
{
	dumper = dumper_;
}

std::shared_ptr<BlokusDumper> Dumpable::getDumper()
{
	return dumper.lock();
}
