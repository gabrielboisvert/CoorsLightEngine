#include <QtCore/qdir.h>
#include <QtCore/qdiriterator.h>

#include "Editor/Utils/Utils.h"
#include <QtWidgets/qmessagebox.h>

using namespace Editor;

QString Utils::uniqueString(const QString& pBase, const char* pExtension)
{
	int idx = -1;
	
	QString path;
	do
	{
		idx++;
		path = pBase + (idx > 0 ? "(" + QString::number(idx) + ")" : "") + "." + pExtension;
	} while (QFile::exists(path));
	
	return path;
}

void Utils::copyAndReplaceFolderContents(const QString& pFromDir, const QString& pToDir, bool pCopyAndRemove)
{
    QDirIterator it(pFromDir, QDirIterator::Subdirectories);
    QDir dir(pFromDir);
    const int absSourcePathLength = dir.absoluteFilePath(pFromDir).length();

    while (it.hasNext())
    {
        it.next();
        const auto fileInfo = it.fileInfo();
        if (!fileInfo.isHidden())
        {
            const QString subPathStructure = fileInfo.absoluteFilePath().mid(absSourcePathLength);
            const QString constructedAbsolutePath = pToDir + subPathStructure;

            if (fileInfo.isDir())
                dir.mkpath(constructedAbsolutePath);
            else if (fileInfo.isFile())
            {
                QFile::remove(constructedAbsolutePath);
                QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
            }
        }
    }

    if (pCopyAndRemove)
        dir.removeRecursively();
}

void Utils::copyFileAndDir(const QString& pFromDir, const QString& pToDir, QWidget* pParent, bool pCopyAndRemove)
{
	if (pFromDir == pToDir)
		return;

	const QString fileName = pFromDir;
	const QString newFileName = fileName.split("/").last();
	const QString newFile = pToDir + "/" + newFileName;

	QFileInfo fileInfos(fileName);
	if (fileInfos.isDir())
	{
		QDirIterator it(fileName, QDirIterator::Subdirectories);
		QDir dir(fileName);
		const int absSourcePathLength = dir.absoluteFilePath(fileName).length();

		while (it.hasNext())
		{
			it.next();
			const QFileInfo fileInfo = it.fileInfo();
			if (!fileInfo.isHidden())
			{
				const QString subPathStructure = fileInfo.absoluteFilePath().mid(absSourcePathLength);
				const QString constructedAbsolutePath = newFile + subPathStructure;

				if (fileInfo.isDir())
					dir.mkpath(constructedAbsolutePath);
				else if (fileInfo.isFile())
				{
					if (QFile::exists(constructedAbsolutePath))
					{
						QMessageBox msgBox(QMessageBox::Question,
							"",
							QString("%1 already exists. \nDo you want to overwrite it?\n").arg(constructedAbsolutePath.split("/").last()),
							QMessageBox::Yes | QMessageBox::No, pParent,
							Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

						if (msgBox.exec() == QMessageBox::Yes)
						{
							QFile::remove(constructedAbsolutePath);
							QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
						}
					}
					else
						QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
				}
			}
		}

		if (pCopyAndRemove)
			dir.removeRecursively();
	}
	else
	{
		if (QFile::exists(newFile))
		{
			QMessageBox msgBox(QMessageBox::Question,
				"",
				QString("%1 already exists. \nDo you want to overwrite it?\n").arg(newFileName),
				QMessageBox::Yes | QMessageBox::No, pParent,
				Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

			if (msgBox.exec() == QMessageBox::Yes)
			{
				QFile::remove(newFile);
				QFile::copy(fileName, newFile);

				if (pCopyAndRemove)
					QFile::remove(fileName);
			}
		}
		else
		{
			QFile::copy(fileName, newFile);

			if (pCopyAndRemove)
				QFile::remove(fileName);
		}
	}
}

std::string Utils::qStringToStdString(const QString& pString)
{
    return pString.toLocal8Bit().constData();
}