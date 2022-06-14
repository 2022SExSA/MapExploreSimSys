#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <random>
#include <set>

#include <QRect>
#include <QPoint>
#include <QDebug>

enum class ReCapPolicy : unsigned char {
    PopFront,
    PopBack,
};

template<typename DataType, std::size_t BlockSize = sizeof(DataType)>
class CircularVector {
public:
    class Iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = DataType;
        using difference_type = std::ptrdiff_t;
        using pointer = DataType*;
        using reference = DataType&;

        Iterator(CircularVector *vec, std::size_t curr)
            : m_vec(vec), m_curr(curr) {
        }

        Iterator &operator++ () {
            ++m_curr;
            return *this;
        }

        Iterator operator++ (int) {
            Iterator old(m_vec, m_curr);
            ++m_curr;
            return old;
        }

        DataType &operator* () const {
            return (*m_vec)[m_curr];
        }

        DataType *operator-> () const {
            return &this->operator*();
        }

        bool operator!= (const Iterator &o) const {
            return m_curr != o.m_curr;
        }
    private:
        CircularVector *m_vec{nullptr};
        std::size_t m_curr{0};
    };
    using data_type = DataType;
    using iterator = Iterator;

    CircularVector(std::size_t cap) : m_baseVec((cap + 1) * BlockSize) {

    }

    template<typename ...ARGS>
    DataType &emplace_back(ARGS&& ...args) {
        DataType *vec = (DataType*)(void*)m_baseVec.data();
        if (this->size() == this->capacity()) {
            vec[m_currBegin].~DataType();
            m_currBegin = (m_currBegin + 1) % this->baseCap();
        }
        new (&vec[m_currEnd]) DataType(std::forward<ARGS>(args)...);
        m_currEnd = (m_currEnd + 1) % this->baseCap();
        return vec[m_currEnd];
    }

    void push_back(const DataType &val) {
        this->emplace_back(val);
    }

    void push_back(DataType &&val) {
        this->emplace_back(std::move(val));
    }

    DataType &operator[] (std::size_t i) {
        DataType *vec = (DataType*)(void*)m_baseVec.data();
        return vec[(m_currBegin + i) % this->baseCap()];
    }

    const DataType &operator[] (std::size_t i) const {
        CircularVector &ncThis = const_cast<CircularVector&>(*this);
        return ncThis[i];
    }

    Iterator begin() {
        return Iterator(this, 0);
    }

    Iterator end() {
        return Iterator(this, this->size());
    }

    std::size_t capacity() const {
        return std::max(this->baseCap() - 1, (decltype(this->baseCap() - 1))0);
    }

    std::size_t size() const {
        return (m_currEnd + this->baseCap() - m_currBegin) % this->baseCap();
    }

    void recap(std::size_t newCap, ReCapPolicy policy = ReCapPolicy::PopBack) {
        const auto oldSize = this->size();
        const auto size = std::min(oldSize, newCap);
                std::cerr << m_currBegin << " " << m_currEnd << "\n";
        std::vector<unsigned char> newBuf((newCap + 1) * BlockSize);
        DataType *newVec = (DataType*)(void*)newBuf.data();
        if (policy == ReCapPolicy::PopBack) {
            for(std::size_t i = 0; i < size; ++i) {
                new (&newVec[i]) DataType(std::move((*this)[i]));
                (*this)[i].~DataType();
            }
            for (std::size_t i = size; i < oldSize; ++i) {
                (*this)[i].~DataType();
            }
        } else if (policy == ReCapPolicy::PopFront) {
            for(std::size_t i = 0; i < size; ++i) {
                const auto ii = i + oldSize - size;
                new (&newVec[i]) DataType(std::move((*this)[ii]));
                (*this)[ii].~DataType();
            }
            for (std::size_t i = 0; i < oldSize - size; ++i) {
                (*this)[i].~DataType();
            }
        } else Q_ASSERT(false);

        m_currBegin = 0;
        m_currEnd = size;
        m_baseVec = std::move(newBuf);
    }

    ~CircularVector() {
        for (auto &e : *this) {
            e.~DataType();
        }
    }
private:
    std::size_t baseCap() const {
        return m_baseVec.size() / BlockSize;
    }
private:
    std::vector<unsigned char> m_baseVec;
    std::size_t m_currBegin{0};
    std::size_t m_currEnd{0};
};

class RandomPositionAllocator {
public:
    RandomPositionAllocator(const QRect & rect, int cellWidth, int cellHeight)
    : m_eng(std::time(nullptr)) {
        m_area = rect;
        m_cellWidth = cellWidth;
        m_cellHeight = cellHeight;

        m_xn = rect.width() / cellWidth;
        m_yn = rect.height() / cellHeight;

        m_gen = std::uniform_int_distribution<int>(0, m_xn * m_yn);
    }

    QPoint next() {
        QPoint res;
        int idx;
        do {
            idx = m_gen(m_eng);
            res.rx() = m_area.x() + (idx % m_xn) * m_cellWidth;
            res.ry() = m_area.y() + (idx / m_xn) * m_cellHeight;
        } while(m_helper.find(idx) != m_helper.end() && m_helper.size() == std::size_t(m_xn * m_yn));
        qDebug() << idx;
        m_helper.insert(idx);
        return res;
    }
private:
    QRect m_area;
    int m_cellWidth, m_cellHeight;
    std::uniform_int_distribution<int> m_gen;
    std::default_random_engine m_eng;
    int m_xn, m_yn;
    std::set<int> m_helper;
};

#endif // UTILS_H

