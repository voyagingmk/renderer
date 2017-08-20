#pragma once

#include <cstddef>
#include <cassert>
#include <vector>
#include <set>  


template <typename T>
class MemoryPool {
protected:
	typedef std::size_t ElementIdx;
public:
	explicit MemoryPool(std::size_t chunkElements = 8192) :
		m_elementSize(sizeof(T)),
		m_chunkElements(chunkElements),
		m_tailIdx(0),
		m_capacity(0) {}

	virtual ~MemoryPool() {
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
			printf("m_capacity -> %d\n", m_capacity);
		}
	}

	inline void* get(ElementIdx idx) {
		assert(idx < m_capacity);
		return m_blocks[idx / m_chunkElements] + m_elementSize * (idx % m_chunkElements);
	}

	inline const T* get(ElementIdx idx) const {
		assert(idx < m_capacity);
		return (static_cast<T*>) m_blocks[idx / m_chunkElements] + m_elementSize * (idx % m_chunkElements);
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
			elem = static_cast<T*>(get(idx));
		}
		return elem;
	}

	ElementIdx dispatchIdx() {
		ElementIdx idx;
		if (recycledIdxes.size() > 0) {
			idx = *recycledIdxes.begin();
			recycled.erase(recycledIdxes.begin());
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

	bool recycle(T* ptr) {
		auto ret = recycledPtrs.insert(ptr);
		return ret.second == true;
	}

	bool recycle(ElementIdx idx) {
		auto ret = recycledIdxes.insert(idx);
		return ret.second == true;
	}

	// No ElementIdx

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

	// Need ElementIdx

	void deleteElement(ElementIdx idx) {
		bool ok = recycle(idx);
		if (!ok) {
			return;
		}
		T* ptr = get(idx);
		ptr->~T();
	}


	template <class... Args>
	T* newElement(ElementIdx idx, Args&&... args) {
		return ::new(get(idx)) T(std::forward<Args>(args) ...);
	}

protected:
	std::set<T*> recycledPtrs;
	std::set<ElementIdx> recycledIdxes;
	std::vector<char *> m_blocks;
	std::size_t m_elementSize;
	std::size_t m_chunkElements;
	std::size_t m_capacity;
	ElementIdx m_tailIdx; // the tail elememnt
};
