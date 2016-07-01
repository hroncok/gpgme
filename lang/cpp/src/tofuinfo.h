/*
  tofuinfo.h - wraps gpgme tofu info
  Copyright (C) 2016 Intevation GmbH

  This file is part of GPGME++.

  GPGME++ is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  GPGME++ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with GPGME++; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef __GPGMEPP_TOFUINFO_H__
#define __GPGMEPP_TOFUINFO_H__

#include "gpgmepp_export.h"

#include "gpgme.h"

#include "global.h"

#include <memory>

namespace GpgME
{

class GPGMEPP_EXPORT TofuInfo
{
public:
    TofuInfo();
    explicit TofuInfo(gpgme_tofu_info_t info);

    const TofuInfo &operator=(TofuInfo other)
    {
        swap(other);
        return *this;
    }

    void swap(TofuInfo &other)
    {
        using std::swap;
        swap(this->d, other.d);
    }

    bool isNull() const;

    /* @enum Validity
     * @brief The TOFU Validity. */
    enum Validity {
        /*! Unknown (uninitialized).*/
        ValidityUnknown,
        /*! TOFU Conflict.*/
        Conflict,
        /*! Key without history.*/
        NoHistory,
        /*! Key with too little history.*/
        LittleHistory,
        /*! Key with enough history for basic trust.*/
        BasicHistory,
        /*! Key with a lot of history.*/
        LargeHistory,
    };
    Validity validity() const;

    /* @enum Policy
     * @brief The TOFU Validity. */
    enum Policy {
        /*! GPGME_TOFU_POLICY_NONE */
        PolicyNone,
        /*! GPGME_TOFU_POLICY_AUTO */
        PolicyAuto,
        /*! GPGME_TOFU_POLICY_GOOD */
        PolicyGood,
        /*! GPGME_TOFU_POLICY_UNKNOWN */
        PolicyUnknown,
        /*! GPGME_TOFU_POLICY_BAD */
        PolicyBad,
        /*! GPGME_TOFU_POLICY_ASK */
        PolicyAsk,
    };
    Policy policy() const;

    /* Number of signatures seen for this binding.  Capped at USHRT_MAX.  */
    unsigned short signCount() const;

    /* Number of seconds since the first message was verified. */
    unsigned int firstSeen() const;

    /* Number of seconds since the last message was verified. */
    unsigned int lastSeen() const;

    /* Finterprint of the key for this entry. */
    const char *fingerprint() const;

    /* If non-NULL a human readable string summarizing the TOFU data. */
    const char *description() const;

    /* The address of the tofu binding.
     *
     * If no mail address is set for a User ID this is the name used
     * for the user ID. Can be ambiguous when the same mail address or
     * name is used in multiple user ids.
     */
    const char *address() const;

private:
    class Private;
    std::shared_ptr<Private> d;
};

GPGMEPP_EXPORT std::ostream &operator<<(std::ostream &os, const TofuInfo &info);

} // namespace GpgME

GPGMEPP_MAKE_STD_SWAP_SPECIALIZATION(TofuInfo)
#endif // __GPGMEPP_TOFUINFO_H__
