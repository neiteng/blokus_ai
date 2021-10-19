#ifndef DUMPABLE_HPP
#define DUMPABLE_HPP

#include <memory>

class BlokusDumper;

class Dumpable
{
	private:
		std::weak_ptr<BlokusDumper> dumper;

	public:
		void setDumper(std::shared_ptr<BlokusDumper> dumper_);
		std::shared_ptr<BlokusDumper> getDumper();
};

#endif
