/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
** 
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QDOCUMENT_LINE_P_H_
#define _QDOCUMENT_LINE_P_H_

#include "qce-config.h"

/*!
	\file qdocumentline_p.h
	\brief Definition of QDocumentLineHandle
*/

#include "qnfa.h"

#include "qformat.h"

#include "qdocumentline.h"

#include <QPair>
#include <QList>
#include <QString>
#include <QVector>

#include <QTextLayout>

#include <QReadWriteLock>

#if QT_VERSION < 0x040400
#include <QAtomic>
#else
#include <QAtomicInt>
#endif


class QPoint;

class QDocument;
class QDocumentLine;
class QDocumentBuffer;
class QDocumentPrivate;
class RenderRange;

class QCE_EXPORT QDocumentLineHandle
{
	friend class QDocument;
	friend class QDocumentLine;
	friend class QDocumentBuffer;
	friend class QDocumentPrivate;
	
	public:
		QDocumentLineHandle(QDocument *d);
		QDocumentLineHandle(const QString& s, QDocument *d);
		
		int count() const;
		int length() const;
		
		int position() const;
		
		QString text() const;
		
		int line() const;
		
		int xToCursor(int x) const;
		int cursorToX(int i) const;
		int cursorToXNoLock(int i) const;
		
		int wrappedLineForCursor(int cpos) const;
		int wrappedLineForCursorNoLock(int cpos) const;
		
		int documentOffsetToCursor(int x, int y) const;
		void cursorToDocumentOffset(int cpos, int& x, int& y) const;
		
		QPoint cursorToDocumentOffset(int cpos) const;
		
		int indent() const;
		
		int nextNonSpaceChar(uint pos) const;
		int previousNonSpaceChar(int pos) const;
		int nextNonSpaceCharNoLock(uint pos) const;
		int previousNonSpaceCharNoLock(int pos) const;
		
		bool hasFlag(int flag) const;
		void setFlag(int flag, bool y = true) const;
		
		QDocument* document() const;
		
		QDocumentLineHandle* next() const;
		QDocumentLineHandle* previous() const;
		
		void updateWrap() const;
		void updateWrapAndNotifyDocument(int ownLineNumber) const;
		
		void setFormats(const QVector<int>& formats);
		
		void clearOverlays();
		void clearOverlays(int format);
		void addOverlay(const QFormatRange& over);
		void addOverlayNoLock(const QFormatRange& over);
		void removeOverlay(const QFormatRange& over);
		bool hasOverlay(int id);
		QList<QFormatRange> getOverlays(int preferredFormat);
		QFormatRange getOverlayAt(int index, int preferredFormat);
		QFormatRange getFirstOverlayBetween(int start, int end, int preferredFormat = -1);
		QFormatRange getLastOverlayBetween(int start, int end, int preferredFormat = -1);
		
		void shiftOverlays(int position, int offset);
		
		void draw(	QPainter *p,
					int xOffset,
					int vWidth,
					const QVector<int>& sel,
					const QPalette& pal,
					bool fullSel,
					int yStart=0,
					int yEnd=-1) const;
		
		QString exportAsHtml(int fromOffset=0, int toOffset = -1) const;

		inline QString& textBuffer() { setFlag(QDocumentLine::LayoutDirty, true); return m_text; }
		
		inline void ref() { m_ref.ref(); }
		inline void deref() { if ( m_ref ) m_ref.deref(); if ( !m_ref ) delete this; }
		int getRef(){ return m_ref; }

		QList<int> getBreaks();
		void clearFrontiers(){
		    m_frontiers.clear();
		}
		
		~QDocumentLineHandle();

                QVector<int> compose() const;
		QVector<int> getFormats() const;

		void lockForRead() const {
		    mLock.lockForRead();
		}
		void lockForWriteText() {
		    mLock.lockForWrite();
		    mTicket++;
		}
		void lockForWrite() {
		    mLock.lockForWrite();
		}
		void unlock() const {
		    mLock.unlock();
		}

		int getCurrentTicket(){
		    return mTicket;
		}

		QVariant getCookie(int type) const{ //locking needs to be done externally !!!
			return mCookies.value(type,QVariant());
		}
		void setCookie(int type,QVariant data){ //locking needs to be done externally !!!
			mCookies.insert(type,data);
		}
		bool hasCookie(int type) const{
			return mCookies.contains(type);
		}
		bool removeCookie(int type){
			return mCookies.remove(type);
		}

	private:
		void layout() const;
		void applyOverlays() const;
		void splitAtFormatChanges(QList<RenderRange>* ranges, const QVector<int>* sel = 0, int from = 0, int until = -1) const;
		
		int getPictureCookieHeight() const;

		QList<QTextLayout::FormatRange> decorations() const;
		
		QString m_text;
		QDocument *m_doc;
#if QT_VERSION < 0x040400
		QBasicAtomic m_ref;
#else
		QAtomicInt m_ref;
#endif
		mutable int m_indent;
		mutable quint16 m_state;
		mutable QTextLayout *m_layout;
		mutable QVector<int> m_cache;
		mutable QVector< QPair<int, int> > m_frontiers;
		
		QNFAMatchContext m_context;
		
		QVector<int> m_formats;
		QVector<QParenthesis> m_parens;
		QList<QFormatRange> m_overlays;

		enum SelectionState {noSel,partialSel,fullSel};
		SelectionState lineHasSelection;
		QBitmap wv;
		mutable QReadWriteLock mLock;
		int mTicket; // increment on each write access to detect obsolete info in parallel thread
		QMap<int,QVariant> mCookies; // store additional info on lines. Helpful for to retrieve info on multiline commands
};

Q_DECLARE_TYPEINFO(QDocumentLineHandle*, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(QDocumentLineHandle*)


#endif // !_QDOCUMENT_LINE_P_H_
