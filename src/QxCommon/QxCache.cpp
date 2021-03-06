/****************************************************************************
**
** http://www.qxorm.com/
** http://sourceforge.net/projects/qxorm/
** Original file by Lionel Marty
**
** This file is part of the QxOrm library
**
** This software is provided 'as-is', without any express or implied
** warranty. In no event will the authors be held liable for any
** damages arising from the use of this software.
**
** GNU Lesser General Public License Usage
** This file must be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file 'license.lgpl.txt' included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you have questions regarding the use of this file, please contact :
** contact@qxorm.com
**
****************************************************************************/

#include <QxPrecompiled.h>

#include <QxCommon/QxCache.h>

#include <QxMemLeak/mem_leak.h>

QX_DLL_EXPORT_QX_SINGLETON_CPP(qx::cache::detail::QxCache)

namespace qx {
namespace cache {
namespace detail {

QxCache::QxCache() : qx::QxSingleton<QxCache>("qx::cache::detail::QxCache"), m_lMaxCost(999999999), m_lCurrCost(0) { ; }

QxCache::~QxCache() { ; }

long QxCache::getCurrCost() const { return m_lCurrCost; }

long QxCache::getMaxCost() const { return m_lMaxCost; }

long QxCache::count() const { return m_cache.count(); }

long QxCache::size() const { return this->count(); }

bool QxCache::isEmpty() const { return (this->count() == 0); }

bool QxCache::exist(const QString & sKey) const { return m_cache.exist(sKey); }

bool QxCache::contains(const QString & sKey) const { return this->exist(sKey); }

void QxCache::setMaxCost(long l)
{
   QMutexLocker locker(& m_oMutexCache);
   m_lMaxCost = ((l < 0) ? 0 : l);
   updateCost();
}

boost::any QxCache::at(const QString & sKey)
{
   QMutexLocker locker(& m_oMutexCache);
   if (! this->exist(sKey)) { return boost::any(); }
   return m_cache.getByKey(sKey).get<2>();
}

long QxCache::insertionCost(const QString & sKey)
{
   QMutexLocker locker(& m_oMutexCache);
   if (! this->exist(sKey)) { return -1; }
   return m_cache.getByKey(sKey).get<0>();
}

QDateTime QxCache::insertionDateTime(const QString & sKey)
{
   QMutexLocker locker(& m_oMutexCache);
   if (! this->exist(sKey)) { return QDateTime(); }
   return m_cache.getByKey(sKey).get<1>();
}

void QxCache::clear()
{
   QMutexLocker locker(& m_oMutexCache);
   m_cache.clear();
   m_lCurrCost = 0;
}

bool QxCache::insert(const QString & sKey, const boost::any & anyObj, long lCost /* = 1 */, const QDateTime & dt /* = QDateTime() */)
{
   if (sKey.isEmpty()) { qAssert(false); return false; }
   this->remove(sKey);

   QMutexLocker locker(& m_oMutexCache);
   lCost = ((lCost < 0) ? 0 : lCost);
   QDateTime dtTemp(dt); if (! dtTemp.isValid()) { dtTemp = QDateTime::currentDateTime(); }
   QxCache::type_qx_cache item = boost::make_tuple(lCost, dtTemp, anyObj);
   bool bInsertOk = m_cache.insert(sKey, item);
   if (bInsertOk) { m_lCurrCost += lCost; updateCost(); }

   return bInsertOk;
}

bool QxCache::remove(const QString & sKey)
{
   QMutexLocker locker(& m_oMutexCache);
   if (! this->exist(sKey)) { return false; }
   long lCost = m_cache.getByKey(sKey).get<0>();
   bool bRemoveOk = m_cache.removeByKey(sKey);
   if (bRemoveOk) { m_lCurrCost -= lCost; }

   return bRemoveOk;
}

void QxCache::updateCost()
{
   while ((m_lCurrCost > m_lMaxCost) && (m_cache.count() > 0))
   {
      QString sKey = m_cache.getKeyByIndex(0);
      long lCost = m_cache.getByIndex(0).get<0>();
      m_cache.removeByIndex(0);
      m_lCurrCost -= lCost;
      QString sMsg = QString("qx::cache : auto remove object in cache '") + sKey + QString("'");
      qDebug("[QxOrm] %s", qPrintable(sMsg));
   }
}

} // namespace detail
} // namespace cache
} // namespace qx
