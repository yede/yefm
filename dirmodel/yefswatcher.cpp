#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <QDebug>

#include "yefsdirmodel.h"
#include "yefswatcher.h"
#include "yefsnode.h"
//==============================================================================================================================

#define use_dbg 0

#if use_dbg
# define dbg(...) qDebug(__VA_ARGS__)
#else
# define dbg(...)
#endif //USE_DEBUG
//==============================================================================================================================

FsWatcher::FsWatcher(FsDirModel *parent)
	: QObject(parent)
	, m_engine(parent)
{
	init();
}

FsWatcher::~FsWatcher()
{
	unregisterWatcher();
}

void FsWatcher::init()
{
	m_halt     = false;
	m_notifier = NULL;
	m_timer    = NULL;
	m_watchId  = 0;

	m_notifyFD = inotify_init();
	if (m_notifyFD < 0) {
		m_halt = true;
		qDebug() << "FsModelEngine::init(): halt" << m_engine->name();
		return;
	}

	m_notifier = new QSocketNotifier(m_notifyFD, QSocketNotifier::Read, this);
	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);

	connect(m_notifier, SIGNAL(activated(int)), this, SLOT(notify(int)));
	connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}
//==============================================================================================================================

bool FsWatcher::haltWarning() const
{
	if (m_halt) {
		qDebug("FsModelEngine::haltWarning(): %s FAILED inotify_init().", m_engine->name());
	}
	return m_halt;
}

void FsWatcher::registerWatcher(const QString &path)
{
	if (m_watchId > 0) unregisterWatcher();

	quint32 mask = IN_MOVE | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MODIFY | IN_ATTRIB;
	m_watchId = inotify_add_watch(m_notifyFD, path.toUtf8().constData(), mask);	// start from 1
	dbg("FsModelEngine::registerWatcher(): %s watchId=%d, path=\"%s\"", m_engine->name(), m_watchId, qPrintable(path));
}

void FsWatcher::unregisterWatcher()
{
	if (m_watchId == 0) return;

	removeWatcher(m_watchId);
	m_watchId = 0;
}

void FsWatcher::removeWatcher(int watchId)
{
	dbg("FsModelEngine::removeWatcher(): %s watchId=%d", m_engine->name(), watchId);
	inotify_rm_watch(m_notifyFD, watchId);
}
//==============================================================================================================================

void FsWatcher::dumpEvent(const inotify_event *event)
{
	static int i = 0;
	QString mask;

	if (event->mask & IN_ACCESS)        mask.append("  IN_ACCESS");
	if (event->mask & IN_MODIFY)        mask.append("  IN_MODIFY");
	if (event->mask & IN_ATTRIB)        mask.append("  IN_ATTRIB");

	if (event->mask & IN_CLOSE)         mask.append("  IN_CLOSE");
	if (event->mask & IN_CLOSE_WRITE)   mask.append("  IN_CLOSE_WRITE");
	if (event->mask & IN_CLOSE_NOWRITE) mask.append("  IN_CLOSE_NOWRITE");

	if (event->mask & IN_OPEN)          mask.append("  IN_OPEN");

	if (event->mask & IN_MOVE)          mask.append("  IN_MOVE");
	if (event->mask & IN_MOVED_FROM)    mask.append("  IN_MOVED_FROM");
	if (event->mask & IN_MOVED_TO)      mask.append("  IN_MOVED_TO");

	if (event->mask & IN_CREATE)        mask.append("  IN_CREATE");
	if (event->mask & IN_DELETE)        mask.append("  IN_DELETE");
	if (event->mask & IN_DELETE_SELF)   mask.append("  IN_DELETE_SELF");
	if (event->mask & IN_MOVE_SELF)     mask.append("  IN_MOVE_SELF");

	if (event->mask & IN_UNMOUNT)       mask.append("  IN_UNMOUNT");
	if (event->mask & IN_Q_OVERFLOW)    mask.append("  IN_Q_OVERFLOW");
	if (event->mask & IN_IGNORED)       mask.append("  IN_IGNORED");

	if (event->mask & IN_ONLYDIR)       mask.append("  IN_ONLYDIR");

	if (event->mask & IN_DONT_FOLLOW)   mask.append("  IN_DONT_FOLLOW");
	if (event->mask & IN_EXCL_UNLINK)   mask.append("  IN_EXCL_UNLINK");

	if (event->mask & IN_MASK_ADD)      mask.append("  IN_MASK_ADD");

	if (event->mask & IN_ISDIR)         mask.append("  IN_ISDIR");
	if (event->mask & IN_ONESHOT)       mask.append("  IN_ONESHOT");

	QString time = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
	qDebug("\n[%d] %s ------------------------- %s", i++, qPrintable(time), m_engine->name());
	qDebug("wd    : %d", event->wd);
	qDebug("mask  : 0x%04x%s", event->mask, mask.toUtf8().toLower().constData());
	qDebug("cookie: %d", event->cookie);
	qDebug("len   : %d", event->len);
	qDebug("name  : %s", event->name);
}

static bool findCookie(int cookie, QList<QString> &fnames, QList<int> &fcookies, QString &fromName)
{
	if (cookie < 1) return false;

	int pos = fcookies.indexOf(cookie);
	if (pos < 0) return false;

	fromName = fnames.takeAt(pos);
	fcookies.removeAt(pos);

	return true;
}

void FsWatcher::notify(int socket)
{
	Q_UNUSED(socket);

	m_notifier->setEnabled(false);
	//--------------------------------------------------------------------------------------------------------------------------

	int buffSize = 0;
	ioctl(m_notifyFD, FIONREAD, (char *) &buffSize);

	QByteArray buffer;
	buffer.resize(buffSize);
	int ret = read(m_notifyFD, buffer.data(), buffSize);
	if (ret < 1) {
		qDebug() << "FsModelEngine::notify(): ret < 1, ret=" << ret;
		return;
	}

	const char *p = buffer.data();
	const char *const end = p + buffSize;
	int cookie;
	QString name, fromName;
	QList<QString> fnames;
	QList<int    > fcookies;

	while (p < end) {
		const inotify_event *event = reinterpret_cast<const inotify_event *>(p);
	//	dumpEvent(event);
		//----------------------------------------------------------------------------------------------------------------------

		if (m_watchId != event->wd)       { removeWatcher(event->wd); goto next; }
		if (event->mask & IN_IGNORED)     {                           goto next; }
		if (event->mask & IN_MOVE_SELF)   { m_engine->tryGoUp();       goto next; }
		if (event->mask & IN_DELETE_SELF) { m_engine->tryGoUp();       goto next; }

		name = QString::fromUtf8(event->name);
		cookie = event->cookie;

		if (event->mask & IN_MOVED_FROM) {
			fnames.append(name);
			fcookies.append(cookie);
			dbg("FsModelEngine::notify(): %s in_moved_from, cookie=%d, name=%s", m_engine->name(), cookie, event->name);
			goto next;
		}

		if (event->mask & IN_MOVED_TO) {
			dbg("FsModelEngine::notify(): %s in_moved_to, cookie=%d, name=%s", m_engine->name(), cookie, event->name);
			if (findCookie(cookie, fnames, fcookies, fromName)) {
				dbg("FsModelEngine::notify(): %s in_moved_to, renameNode: \"%s\" -> \"%s\"", m_engine->name(), qPrintable(fromName), qPrintable(name));
				m_engine->renameNode(fromName, name);
			} else {
				dbg("FsModelEngine::notify(): %s in_moved_to, insertNode: \"%s\"", m_engine->name(), qPrintable(name));
				m_engine->insertNode(name);
			}
			goto next;
		}

		if (event->mask & IN_ATTRIB) { m_engine->updateNode(name); goto next; }
		if (event->mask & IN_MODIFY) { m_engine->updateNode(name); goto next; }
		if (event->mask & IN_CREATE) { m_engine->insertNode(name); goto next; }
		if (event->mask & IN_DELETE) { m_engine->deleteNode(name); goto next; }
		//----------------------------------------------------------------------------------------------------------------------
next:
		p += sizeof(inotify_event) + event->len;
	}
	//--------------------------------------------------------------------------------------------------------------------------

	while (fnames.size() > 0) {
		name = fnames.takeAt(0);
		cookie = fcookies.takeAt(0);
		dbg("FsModelEngine::notify(): %s outside-loop, cookie=%d, name=%s", m_engine->name(), cookie, qPrintable(name));
		m_engine->deleteNode(name);
	}
	//--------------------------------------------------------------------------------------------------------------------------

	m_notifier->setEnabled(true);
}
//==============================================================================================================================

void FsWatcher::timeout()
{
}
//==============================================================================================================================
