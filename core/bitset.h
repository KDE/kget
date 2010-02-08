/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTBITSET_H
#define BTBITSET_H

#include <kget_export.h>

/**
  * @author Joris Guisson
  * @brief Simple implementation of a BitSet
  * 
  * Simple implementation of a BitSet, can only turn on and off bits.
  * BitSet's are used to indicate which chunks we have or not.
  */
class KGET_EXPORT BitSet
{
	quint32 num_bits,num_bytes;
	quint8* data;
	quint32 num_on;
public:
	/**
	  * Constructor.
	  * @param num_bits The number of bits
	  */
	BitSet(quint32 num_bits = 8);
	
	/**
	  * Manually set data.
	  * @param data The data
	  * @param num_bits The number of bits
	  */
	BitSet(const quint8* data,quint32 num_bits);
	
	/**
	  * Copy constructor.
	  * @param bs BitSet to copy
	  * @return 
	  */
	BitSet(const BitSet & bs);
	virtual ~BitSet();

	/// See if the BitSet is null
	bool isNull() const {return num_bits == 0;}
	
	/**
	  * Get the value of a bit, false means 0, true 1.
	  * @param i Index of Bit
	  */
	bool get(quint32 i) const;
	
	/**
	  * Set the value of a bit, false means 0, true 1.
	  * @param i Index of Bit
	  * @param on False means 0, true 1
	  */
	void set(quint32 i,bool on);

    /**
     * Sets the value of a range of bits
     * @param start of the range
     * @param end of the range
     * @param value to set the range to
     */
    void setRange(quint32 start, quint32 end, bool value);
	
	/// Set all bits on or off
	void setAll(bool on);
	
	quint32 getNumBytes() const {return num_bytes;}
	quint32 getNumBits() const {return num_bits;}
	const quint8* getData() const {return data;}
	quint8* getData() {return data;}

	/// Get the number of on bits
	quint32 numOnBits() const {return num_on;}

    /**
     * Finds a continous rang of bits that on/off
     * @param start here the start bit will be stored, -1 if nothing is found
     * @param end stores the end bit, -1 if nothing is found
     * @param on wether a continous range of bits on (set) or off (not set) should be searched for
     */
    void getContinuousRange(qint32 *start, qint32 *end, bool on);
	
	/**
	  * Set all bits to 0
	  */
	void clear();

	/**
	  * or this BitSet with another.
	  * @param other The other BitSet
	  */
	void orBitSet(const BitSet & other);
	
	/**
	  * Assignment operator.
	  * @param bs BitSet to copy
	  * @return *this
	  */
	BitSet & operator = (const BitSet & bs);

	/// Check if all bit are set to 1
	bool allOn() const;
    bool allOff() const;

	/**
	  * Check for equality of bitsets
	  * @param bs BitSet to compare
	  * @return true if equal 
	  */
	bool operator == (const BitSet & bs);
	
	/**
	  * Opposite of operator == 
	  */
	bool operator != (const BitSet & bs) {return ! operator == (bs);}

	static BitSet null;
};

inline bool BitSet::get(quint32 i) const
{
	if (i >= num_bits)
		return false;
	
	quint32 byte = i / 8;
	quint32 bit = i % 8;
	quint8 b = data[byte] & (0x01 << (7 - bit));
	return b != 0x00;
}

inline void BitSet::set(quint32 i,bool on)
{
	if (i >= num_bits)
		return;
	
	quint32 byte = i / 8;
	quint32 bit = i % 8;
	if (on && !get(i))
	{
		num_on++;
		data[byte] |= (0x01 << (7 - bit));
	}
	else if (!on && get(i))
	{
		num_on--;
		quint8 b = (0x01 << (7 - bit));
		data[byte] &= (~b);
	}
}

inline void BitSet::setRange(quint32 start, quint32 end, bool value)
{
    if ((start >= num_bits) || (end >= num_bits)) {
        return;
    }

    for (quint32 i = start; i <= end; ++i) {
        set(i, value);
    }
}

#endif
