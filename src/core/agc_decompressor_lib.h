#ifndef _AGC_DECOMPRESSOR_LIB_H
#define _AGC_DECOMPRESSOR_LIB_H

// *******************************************************************************************
// This file is a part of AGC software distributed under MIT license.
// The homepage of the AGC project is https://github.com/refresh-bio/agc
//
// Copyright(C) 2021-2024, S.Deorowicz, A.Danek, H.Li
//
// Version: 3.1
// Date   : 2024-03-12
// *******************************************************************************************

#include <regex>
#include "../core/agc_basic.h"

// *******************************************************************************************
// Class supporting only decompression of AGC files - library version
class CAGCDecompressorLibrary : public CAGCBasic
{
protected:
	const uint8_t cnv_num[128] = {
		//		0    1	   2    3    4    5    6    7    8    9    10   11   12  13   14    15
				'A', 'C', 'G', 'T', 'N', 'R', 'Y', 'S', 'W', 'K', 'M', 'B', 'D', 'H', 'V', 'U',
				' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
				' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
				' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
				' ',   0,  11,   1,  12,  30,  30,   2,  13,  30,  30,   9,  30,  10,   4,  30,
				 30,  30,   5,   7,   3,  15,  14,   8,  30,   6,  30,  30,  30,  30,  30,  30,
				' ',   0,  11,   1,  12,  30,  30,   2,  13,  30,  30,   9,  30,  10,   4,  30,
				 30,  30,   5,   7,   3,  15,  14,   8,  30,   6,  30,  30,  30,  30,  30,  30
	};

	// *******************************************************************************************
	struct name_range_t
	{
		string name;
		int64_t from;
		int64_t to;

		name_range_t(const string _name = "", const int64_t _from = -1, const int64_t _to = -1) :
			name(_name), from(_from), to(_to)
		{}

		string str() const
		{
			if (from >= 0 && to >= 0)
				return name + ":" + to_string(from) + "-" + to_string(to);
			else
				return name;
		}
	};

	const regex re_csr = regex("(.+)@(.+):(.+)-(.+)");
	const regex re_cs = regex("(.+)@(.+)");
	const regex re_cr = regex("(.+):(.+)-(.+)");
	const regex re_c = regex("(.+)");

	struct sample_contig_data_t {
		string sample_name;
		string contig_name;
		contig_t contig_data;

		sample_contig_data_t() = default;
		sample_contig_data_t(string _sample_name, string _contig_name, contig_t _contig_data) :
			sample_name(_sample_name), contig_name(_contig_name), contig_data(_contig_data) {}

		sample_contig_data_t(const sample_contig_data_t&) = default;
		sample_contig_data_t(sample_contig_data_t&&) = default;
		sample_contig_data_t& operator=(const sample_contig_data_t&) = default;
	};

	struct contig_task_t {
		size_t priority;
		string sample_name;
		name_range_t name_range;
		vector<segment_desc_t> segments;

		contig_task_t() = default;
		contig_task_t(const size_t _priority, const string _sample_name, const name_range_t _name_range, const vector<segment_desc_t>& _segments) :
			priority(_priority), sample_name(_sample_name), name_range(_name_range), segments(_segments) {};
		contig_task_t(const contig_task_t&) = default;
		contig_task_t(contig_task_t&&) = default;
		contig_task_t& operator=(const contig_task_t&) = default;
	};

	unique_ptr<CBoundedQueue<contig_task_t>> q_contig_tasks;
	unique_ptr<CPriorityQueue<sample_contig_data_t>> pq_contigs_to_save;

	void convert_to_alpha(contig_t& ctg);

	bool analyze_contig_query(const string& query, string& sample, name_range_t& name_range);
	bool decompress_segment(const uint32_t group_id, const uint32_t in_group_id, contig_t& ctg, ZSTD_DCtx* zstd_ctx);

	bool decompress_contig(contig_task_t& task, ZSTD_DCtx *zstd_ctx, contig_t& ctg);

	bool close_decompression();

public:
	CAGCDecompressorLibrary(bool _is_app_mode);
	~CAGCDecompressorLibrary();

	bool Open(const string& _archive_fn, const bool _prefetch_archive = false);

	void GetCmdLines(vector<pair<string, string>>& _cmd_lines);
	void GetParams(uint32_t& kmer_length, uint32_t& min_match_len, uint32_t& pack_cardinality);
	void GetReferenceSample(string& ref_name);

	bool Close();

	int GetContigString(const string& sample_name, const string& contig_name, const int start, const int end, string& contig_data);
	int64_t GetContigLength(const string& sample_name, const string& contig_name);

	bool ListSamples(vector<string>& v_sample_names);
	bool ListContigs(const string& sample_name, vector<string>& v_contig_names);
	int32_t GetNoSamples();
	int32_t GetNoContigs(const string& sample_name);

	void GetFileTypeInfo(map<string, string>& _m_file_type_info);

	bool IsOpened();
};

// EOF
#endif