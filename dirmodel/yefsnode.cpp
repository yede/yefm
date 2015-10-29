#include <QDir>
#include <QFileInfoList>
#include <QDebug>

#include "yefsnode.h"
#include "yeappcfg.h"
#include "yemime.h"
//==============================================================================================================================

FsNode::FsNode(const QFileInfo &fileInfo)
	: m_fileInfo(fileInfo)
	, m_bools(0)
{
}

FsNode::FsNode()
	: m_bools(0)
{
}

FsNode::~FsNode()
{
}
//==============================================================================================================================

const QString &FsNode::getMimeType()
{
	if (m_mimeType.isNull()) {
		m_mimeType = Mime::instance()->getMimeType(m_fileInfo.filePath());
	}

	return m_mimeType;
}

const QString &FsNode::getPermissions()
{
	if (m_permissions.isNull())
	{
#if QT_VERSION >= 0x050000
		QFile::Permissions perms = m_fileInfo.permissions();
		m_permissions.append(perms.testFlag(QFileDevice::ReadOwner ) ? "r" : "-");
		m_permissions.append(perms.testFlag(QFileDevice::WriteOwner) ? "w" : "-");
		m_permissions.append(perms.testFlag(QFileDevice::ExeOwner  ) ? "x" : "-");
		m_permissions.append(perms.testFlag(QFileDevice::ReadGroup ) ? "r" : "-");
		m_permissions.append(perms.testFlag(QFileDevice::WriteGroup) ? "w" : "-");
		m_permissions.append(perms.testFlag(QFileDevice::ExeGroup  ) ? "x" : "-");
		m_permissions.append(perms.testFlag(QFileDevice::ReadOther ) ? "r" : "-");
		m_permissions.append(perms.testFlag(QFileDevice::WriteOther) ? "w" : "-");
		m_permissions.append(perms.testFlag(QFileDevice::ExeOther  ) ? "x" : "-");
#else
		QFlags<QFile::Permissions> perms = m_fileInfo.permissions();
		m_permissions.append(perms.testFlag(QFile::ReadOwner ) ? "r" : "-");
		m_permissions.append(perms.testFlag(QFile::WriteOwner) ? "w" : "-");
		m_permissions.append(perms.testFlag(QFile::ExeOwner  ) ? "x" : "-");
		m_permissions.append(perms.testFlag(QFile::ReadGroup ) ? "r" : "-");
		m_permissions.append(perms.testFlag(QFile::WriteGroup) ? "w" : "-");
		m_permissions.append(perms.testFlag(QFile::ExeGroup  ) ? "x" : "-");
		m_permissions.append(perms.testFlag(QFile::ReadOther ) ? "r" : "-");
		m_permissions.append(perms.testFlag(QFile::WriteOther) ? "w" : "-");
		m_permissions.append(perms.testFlag(QFile::ExeOther  ) ? "x" : "-");
#endif
		m_permissions.append(" " + m_fileInfo.owner() + " " + m_fileInfo.group());
	}

	return m_permissions;
}

const QString FsNode::getDataTime()
{
	QDateTime date = m_fileInfo.lastModified();
	return QString("%1 %2")
			.arg(date.toString(AppCfg::instance()->fileDateFormat))
			.arg(date.toString(AppCfg::instance()->fileTimeFormat));
}

const QString FsNode::getFilePath(const QString &newName)
{
	return isRoot() ? (sep + newName) : (m_fileInfo.path() + sep + newName);
}
//==============================================================================================================================

void FsNode::refresh()
{
	m_fileInfo.refresh();
	clear();
}

void FsNode::clear()
{
	m_permissions.clear();
	m_mimeType.clear();
}

void FsNode::clearMimeType()
{
	m_mimeType.clear();
}

void FsNode::setFileName(const QString &fileName)
{
	QString filePath = getFilePath(fileName);
	setFilePath(filePath);
}

void FsNode::setFilePath(const QString &filePath)
{
	m_fileInfo.setFile(filePath);
	refresh();
}
//==============================================================================================================================
