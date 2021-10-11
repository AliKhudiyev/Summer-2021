
#pragma once

#include "defs.h"
#include "system.h"
#include "bit_stream.h"

namespace alc{ 
	class IO{
		public:
		std::string expression;
		virtual raw_output_t eval(const raw_input_t& input) const;
		std::string eval(const std::string& input) const;
		void eval(size_t set_count=-1, bool repetitions=true);

		protected:
		size_t m_input_count = 0, m_output_count = 0;

		private:
		std::vector<BitStream> m_table;
		mutable System m_system;

		public:
		IO() = default;
		~IO() = default;

		void add(const raw_input_t& input, const raw_output_t& output);
		void remove_input(const raw_input_t& input);
		void remove_output(const raw_output_t& output);

		raw_output_t generate(const raw_input_t& input) const;
		std::string generate(const std::string& input) const;
		std::pair<raw_input_t, raw_output_t> generate() const;
		void generate(size_t set_count, size_t output_count=1, bool repetitions=true);
		void generate(size_t input_count, size_t output_count, size_t set_count, 
				size_t min_or_count, size_t max_or_count, float not_freq=0.5f, bool repetitions=true);

		size_t input_count() const;
		size_t output_count() const;
		size_t size() const;
		raw_input_t input_at(size_t index) const;
		raw_output_t output_at(size_t index) const;
		System system() const;

		void input_at(size_t index, const raw_input_t& input);
		void output_at(size_t index, const raw_output_t& output);
		void shuffle();

		void save(const char* file_path) const;
		bool load(const char* file_path);
		void print(FILE* file=stdout) const{
			for(size_t i=0; i<m_table.size(); ++i){
				fprintf(file, "%s -> %s\n", 
						input_at(i).to_string().c_str(), output_at(i).to_string().c_str());
			}
		}

		BitStream& operator[](size_t index);

		private:
		static std::vector<std::shared_ptr<Core>> choose
			(const std::vector<std::shared_ptr<Core>>& cores, size_t n=1){
				auto cores2 = cores;
				std::random_shuffle(cores2.begin(), cores2.end());
				size_t sz = n <= cores.size() ? n : cores.size();
				// printf("%zu vs %zu\n", cores.size(), n);
				std::vector<std::shared_ptr<Core>> output(sz);
				for(size_t i=0; i<sz; ++i)
					output[i] = cores2[i];
				// printf("chose %zu cores\n", sz);
				return output;
		}
	};
}

