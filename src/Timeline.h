#pragma once

namespace VTPlayerLib
{

template < typename T >
struct ITimeline
{
    virtual ~ITimeline() = default;

    virtual uint64_t GetLength() const noexcept = 0;

    virtual T Get(uint64_t pos) const noexcept = 0;
};

template < typename T > using Timeline = std::shared_ptr<ITimeline<T>>;

template < typename T >
class TimelineList : public ITimeline<T>
{
    struct Comparer : IComparer<KeyValuePair<long, T>>
    {
        #pragma region IComparer<KeyValuePair<long, T>> Members

            public int Compare(KeyValuePair<long, T> x, KeyValuePair<long, T> y)
        {
            return x.Key.CompareTo(y.Key);
        }

        #pragma endregion
    }

    List<List<KeyValuePair<long, T>>> data = new List<List<KeyValuePair<long, T>>>();
    T defaultValue;
    int bits = 8;
    uint64_t length = 0;

    uint64_t GetLength() const noexcept override
    {
        return length;
    }

    TimelineList(T defaultValue, int bits)
    {
        this.defaultValue = defaultValue;
        this.bits = bits;
    }

    void Add(uint64_t pos, T value)
    {
        Debug.Assert(pos >= 0);

        if (pos > length)
        {
            length = pos;
        }

        int isecond = (int)(pos >> bits);

        if (isecond >= data.Count)
        {
            data.AddRange(Enumerable.Repeat<List<KeyValuePair<long, T>>>(null, isecond - data.Count + 1));
        }

        if (data[isecond] == null)
        {
            data[isecond] = new List<KeyValuePair<long, T>>(1);
        }

        var list = data[isecond];
        var key = new KeyValuePair<long, T>(pos, value);

        var i = list.BinarySearch(key, new Comparer());
        if (i >= 0)
        {
            list[i] = key;
        }
        else
        {
            list.Insert(~i, key);
        }
    }

    #pragma region ITimeline<T> Members

        public T Get(long pos)
    {
        int isecond = (int)(pos >> bits);

        if (isecond >= data.Count)
        {
            isecond = data.Count - 1;
        }

        if (isecond < 0)
        {
            return defaultValue;
        }

        var key = new KeyValuePair<long, T>(pos, defaultValue);

        while (isecond >= 0)
        {
            var list = data[isecond];

            if (list != null)
            {
                var n = list.Count;
                int i = list.BinarySearch(key, new Comparer());
                if (i >= 0)
                {
                    return list[i].Value;
                }
                else
                {
                    i = (~i) - 1;
                    if (i >= 0)
                    {
                        return list[i].Value;
                    }
                }
            }

            --isecond;
        }

        return defaultValue;
    }

    #pragma endregion
}

}
// namespace VTPlayerLib
