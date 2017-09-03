#ifndef MEMORYPOOL_HPP
#define MEMORYPOOL_HPP

#include <cstddef>
#include <cassert>
#include <vector>
#include <set>  


class MemoryPoolBase {
public:
	typedef std::size_t ElementIdx;
public:
	explicit MemoryPoolBase(std::size_t chunkElements, std::size_t elemSize) :
		m_chunkElements(chunkElements),
		m_elementSize(elemSize),
		m_tailIdx(0),
		m_capacity(0) {}

	virtual ~MemoryPoolBase() {
		for (char *ptr : m_blocks) {
			delete[] ptr;
		}
	}
	std::size_t capacity() const { return m_capacity; }
	std::size_t chunks() const { return m_blocks.size(); }

	inline void reserve(ElementIdx nRequiredElements) {
		while (m_capacity < nRequiredElements) {
			char *chunk = new char[m_elementSize * m_chunkElements];
			m_blocks.push_back(chunk);
			m_capacity += m_chunkElements;
		}
	}

	inline void* getRaw(ElementIdx idx) const {
		assert(idx < m_capacity);
		return m_blocks[idx / m_chunkElements] + m_elementSize * (idx % m_chunkElements);
	}

	ElementIdx dispatchIdx() {
		ElementIdx idx;
		if (recycledIdxes.size() > 0) {
			idx = *recycledIdxes.begin();
			recycledIdxes.erase(recycledIdxes.begin());
		}
		else {
			if (m_tailIdx >= m_capacity) {
				// add 1 chunk
				reserve(m_capacity + m_chunkElements);
			}
			idx = m_tailIdx++;
		}
		return idx;
	}

	bool recycleIdx(ElementIdx idx) {
		auto ret = recycledIdxes.insert(idx);
		return ret.second == true;
	}
	virtual void deleteElementByIdx(ElementIdx idx) = 0;

protected:
	std::set<ElementIdx> recycledIdxes;
	std::vector<char *> m_blocks;
	std::size_t m_elementSize;
	std::size_t m_chunkElements;
	std::size_t m_capacity;
	ElementIdx m_tailIdx; // the tail elememnt
};

template <typename T>
class MemoryPool: public MemoryPoolBase {
public:
	explicit MemoryPool(std::size_t chunkElements = 8192):
		MemoryPoolBase(chunkElements, sizeof(T))
	{
	}

	virtual ~MemoryPool() {
		recycledPtrs.clear();
	}

	inline const T* get(ElementIdx idx) const {
		assert(idx < m_capacity);
		return static_cast<const T*>(getRaw(idx));
	}

	T* dispatchPtr() {
		T* elem;
		if (recycledPtrs.size() > 0) {
			elem = *recycledPtrs.begin();
			recycledPtrs.erase(recycledPtrs.begin());
		}
		else {
			if (m_tailIdx >= m_capacity) {
				// add 1 chunk
				reserve(m_capacity + m_chunkElements);
			}
			ElementIdx idx = m_tailIdx++;
			elem = static_cast<T*>(getRaw(idx));
		}
		return elem;
	}


	void deleteElement(T* ptr) {
		bool ok = recycle(ptr);
		if (!ok) {
			return;
		}
		ptr->~T();
	}


	template <class... Args>
	T* newElement(Args&&... args) {
		return ::new(dispatchPtr()) T(std::forward<Args>(args) ...);
	}

	void deleteElementByIdx(ElementIdx idx) {
		bool ok = recycleIdx(idx);
		if (!ok) {
			return;
		}
		const T* ptr = get(idx);
		ptr->~T();
	}


	template <class... Args>
	T* newElementByIdx(ElementIdx idx, Args&&... args) {
		reserve(idx);
		return ::new(getRaw(idx)) T(std::forward<Args>(args) ...);
	}

	bool recycle(T* ptr) {
		auto ret = recycledPtrs.insert(ptr);
		return ret.second == true;
	}

protected:
	std::set<T*> recycledPtrs;
};

#endif