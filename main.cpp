#include <iostream>

#include <QtCore/QByteArray>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QTextStream>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);

    QCommandLineParser cmdParser;
    //cmdParser.addHelpOption(); // see below
    cmdParser.addOption({{"h", "help", "?"}, "Displays help on commandline options."});
    cmdParser.addOptions({
      {{"e", "existing-path" }, "Existing path to look for.", "existing-path"}
    , {{"n", "new-path"      }, "New path to replace existing path with.", "new-path"}
    , {{"r", "regex"         }, "Existing and New paths are regex patterns."}
    , {{"b", "bt-backup-path"}, "BT_backup folder path (path to directory which stores .fastresume files).", "bt-backup-path"}
    , {"verbose"              , "Print extra information."}
    });
    cmdParser.process(app);

    // need to explicitly print help text, because fucking Qt forcibly limits text width
    if (cmdParser.isSet("h") || argc < 2)
    {
        out << QString(
        "Usage: %1 [options]\n"
        "Example: change all paths containing \"D:/Downloads_old\" to \"D:/Downloads_new\". So e.g. \"D:/Downloads_old/Drive.mkv\" will change to \"D:/Downloads_new/Drive.mkv\"\n"
        "         %1 -e \"D:/Downloads_old\" -n \"D:/Downloads_new\" -b \"C:/qBitTorrent/profile/qBittorrent/data/BT_backup\"\n"
        "Example: use regex to change e.g. \"../../Downloads/Drive.mkv\" to \"F:/Downloads/Drive.mkv\"\n"
        "         %1 -r -e \"^(?:\\.\\./){1,}Downloads/(.+)\" -n \"F:/Downloads/\\1\" -b \"C:/qBitTorrent/profile/qBittorrent/data/BT_backup\"\n"
        "NOTE: %1 converts all \\ backslashes to / slashes, so you can write path unix-style even on Windows\n"
        "\n"
        "Options:\n"
        "  -?, -h, --help                         Displays help on commandline options.\n"
        "  -e, --existing-path <existing-path>    Existing path to look for.\n"
        "  -n, --new-path <new-path>              New path to replace existing path with.\n"
        "  -r, --regex                            Existing and New paths are regex patterns.\n"
        "  -b, --bt-backup-path <bt-backup-path>  BT_backup folder path (path to directory which stores .fastresume files).\n"
        "  --verbose                              Print extra information.\n"
               ).arg(app.applicationName()) << Qt::endl;
        return 0;
    }
    if (!cmdParser.isSet("existing-path"))
    {
        out << "Error! existing-path is not set." << Qt::endl;
        return 0;
    }
    if (!cmdParser.isSet("new-path"))
    {
        out << "Error! new-path is not set." << Qt::endl;
        return 0;
    }
    if (!cmdParser.isSet("bt-backup-path"))
    {
        out << "Error! bt-backup-path is not set." << Qt::endl;
        return 0;
    }

    bool isVerbose = cmdParser.isSet("verbose");
    bool isRegex = cmdParser.isSet("regex");
    QString beforeString(cmdParser.value("existing-path"));
    QRegularExpression regExp(beforeString);
    QString afterString(cmdParser.value("new-path"));
    uint affectedCounter = 0;

    QDir targetDir(cmdParser.value("bt-backup-path"));
    QStringList nameFilters({"*.fastresume"});
    QDir::Filters filters = QDir::Files;
    QDir::SortFlags sortFlags = QDir::Name | QDir::DirsFirst;
    QList<QFileInfo> allFileDirs = targetDir.entryInfoList(nameFilters, filters, sortFlags);
    for (auto iter = allFileDirs.begin(); iter != allFileDirs.end(); ++iter)
    {
        QFile file(iter->canonicalFilePath());
        if (!file.open(QIODevice::ReadWrite))
        {
            out << "Failed to open file " << iter->canonicalFilePath() << Qt::endl;
            continue;
        }

        QTextStream textStream(&file);
        QString line = textStream.readAll();
        const QString qbt_savePath("qBt-savePath");
        const QString savePath("save_path");

        if (isRegex)
        {
            auto lambda = [&line, &regExp, &afterString, &out, &file](const QString& findString, const bool isVerbose = false) mutable {
                QString lengthOriginalString;
                int pos = line.indexOf(findString, 0) + findString.length();
                for (int t = pos; line.at(t).isDigit(); ++t)
                    lengthOriginalString.append(line.at(t));
                QString pathString = line.mid(pos + lengthOriginalString.length() + 1, lengthOriginalString.toInt()); // +1 for ':'
                pathString.replace(QChar('\\'), QChar('/'));
                QString resultingReplace = QString(pathString).replace(regExp, afterString);
                bool isChanged = (pathString != resultingReplace); // check that original path was actually changed
                if (isChanged && isVerbose)
                    out << QFileInfo(file).fileName() << " : \"" << pathString << "\" -> \"" << resultingReplace << "\"" << Qt::endl;
                resultingReplace = QString::number(resultingReplace.length()) + ":" + resultingReplace;
                line.replace(pos, lengthOriginalString.length() + 1 + lengthOriginalString.toInt(), resultingReplace);
                return isChanged;
            };
            if (lambda(qbt_savePath, isVerbose) == false)
                continue;
            lambda(savePath);
        }
        else
        {
            // only differs from above by: regExp -> beforeString
            auto lambda = [&line, &beforeString, &afterString, &out, &file](const QString& findString, const bool isVerbose = false) mutable {
                QString lengthOriginalString;
                int pos = line.indexOf(findString, 0) + findString.length();
                for (int t = pos; line.at(t).isDigit(); ++t)
                    lengthOriginalString.append(line.at(t));
                QString pathString = line.mid(pos + lengthOriginalString.length() + 1, lengthOriginalString.toInt()); // +1 for ':'
                pathString.replace(QChar('\\'), QChar('/'));
                QString resultingReplace = QString(pathString).replace(beforeString, afterString);
                bool isChanged = (pathString != resultingReplace); // check that original path was actually changed
                if (isChanged && isVerbose)
                    out << QFileInfo(file).fileName() << " : \"" << pathString << "\" -> \"" << resultingReplace << "\"" << Qt::endl;
                resultingReplace = QString::number(resultingReplace.length()) + ":" + resultingReplace;
                line.replace(pos, lengthOriginalString.length() + 1 + lengthOriginalString.toInt(), resultingReplace);
                return isChanged;
            };
            if (lambda(qbt_savePath, isVerbose) == false)
                continue;
            lambda(savePath);
        }
        ++affectedCounter;

        QByteArray result = line.toLocal8Bit();
        file.resize(result.size());
        file.seek(0);
        file.write(result);
        file.close();
    }
    out << "Changed " << affectedCounter << " files" << Qt::endl;
    return 0;
}
