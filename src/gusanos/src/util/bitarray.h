#ifndef OMFGUTIL_DETAIL_BITARRAY_H
#define OMFGUTIL_DETAIL_BITARRAY_H

class BitArray
{
public:
	struct Segment
	{
		Segment()
		{
		}
		
		Segment(size_t pos_, size_t len_, bool val_)
		: pos(pos_), len(len_), val(val_)
		{
		}
		
		size_t pos; // Position in the bit array
		size_t len; // Length of this segment
		bool   val; // The value of this segment
	};
	
	typedef std::vector<Segment> SegmentCont;
	typedef SegmentCont::iterator SegmentIter;
	
	struct AccessProxy
	{
		AccessProxy(BitArray& array_, SegmentIter segm_, size_t offset_)
		: array(array_), segm(segm_), offset(offset_)
		{
		}
		
		void operator=(bool v)
		{
			array.set(segm, offset, v);
		}
		
		operator bool()
		{
			return segm->val;
		}
		
		BitArray& array;
		SegmentIter segm;
		size_t offset;
	};
	
	struct Scanner
	{
		Scanner(BitArray& array_, SegmentIter segm_)
		: array(array_), segm(segm_)
		{
			
		}
		
		// Scans forward until the first bit with value v
		bool scanf(bool v)
		{
			if(segm->val == v)
				return true;
			if(segm == array.segments.end() - 1)
			{
				offset = segm->pos + segm->len;
				return false;
			}
			++segm;
			offset = 0;
			assert(segm->val == v);
			return true;
		}
		
		size_t pos()
		{
			return segm->pos + offset;
		}
		
		BitArray& array;
		SegmentIter segm;
		size_t offset;
	};
	
	BitArray(size_t s_)
	: s(s_)
	{
		segments.push_back(Segment(0, s, false));
	}
	
	AccessProxy operator[](size_t pos)
	{
		SegmentIter segm = findSegment(pos);
		return AccessProxy(*this, segm, pos - segm->pos);
	}

	void set(SegmentIter segm, size_t offset, bool val)
	{
		if(val != segm->val)
		{
			if(offset == 0)
			{
				segm->pos = offset + 1;
				--segm->len;
				
				if(segm == segments.begin())
				{
					// We must add a segment at the beginning
					segments.insert(segm, Segment(offset, 1, val));
					// segm is invalid here
				}
				else
				{
					SegmentIter prev = segm - 1;
					++prev->len;
					assert(prev->val == val);
					assert(prev->pos + prev->len == segm->pos);
				}
			}
			else if(offset == segm->len - 1)
			{
				--segm->len;
				if(segm == segments.end() - 1)
				{
					// We must add a segment at the end
					segments.push_back(Segment(offset, 1, val));
				}
				else
				{
					SegmentIter next = segm + 1;
					++prev->len;
					--prev->pos;
					assert(next->val == val);
					assert(segm->pos + segm->len == next->pos);
				}
			}
			else
			{
				segments.insert(segm + 1, 2, Segment());
				size_t basePos = segm->pos;
				size_t baseLen = segm->len;
				segm->len = offset;
				
				++segm;
				segm->pos = basePos + offset;
				segm->len = 1;
				segm->val = val;
				
				++segm;
				segm->pos = basePos + offset + 1;
				segm->len = baseLen - offset - 1;
				segm->val = !val;
			}
		}
	}

	Scanner scanner(size_t pos)
	{
		return Scanner(*this, findSegment(pos));
	}
	
private:
	
	
	SegmentIter findSegment(size_t pos)
	{
		int low = 0;
		int high = segments.size() - 1;
		
		while(low < high)
		{
			int mid = (low + high) / 2;
			if(segments[mid].pos > pos)
				high = mid - 1;
			else if(segments[mid].pos < pos)
				low = mid + 1;
			else
				return segments.begin() + mid;
		}
		
		if(low == high)
		{
			if(segments[low].pos > pos)
			{
				if(low > 0)
					return segments.begin() + (low - 1);
				else
					return segments.end();
			}
			else
				return segments.begin() + low;
		}
		else if(low > high)
		{
			return segments.end();
		}
	}
	
	SegmentCont segments;
	size_t s;
};

#endif //OMFGUTIL_DETAIL_BITARRAY_H
