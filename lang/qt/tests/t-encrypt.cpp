/* t-encrypt.cpp

    This file is part of qgpgme, the Qt API binding for gpgme
    Copyright (c) 2016 Intevation GmbH

    QGpgME is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    QGpgME is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
#include <QDebug>
#include <QTest>
#include <QTemporaryDir>
#include "keylistjob.h"
#include "encryptjob.h"
#include "qgpgmeencryptjob.h"
#include "encryptionresult.h"
#include "decryptionresult.h"
#include "qgpgmedecryptjob.h"
#include "qgpgmebackend.h"
#include "keylistresult.h"
#include "t-support.h"

using namespace QGpgME;
using namespace GpgME;

class EncryptionTest : public QGpgMETest
{
    Q_OBJECT

private Q_SLOTS:

    void testSimpleEncryptDecrypt()
    {
        auto listjob = openpgp()->keyListJob(false, false, false);
        std::vector<Key> keys;
        auto keylistresult = listjob->exec(QStringList() << QStringLiteral("alfa@example.net"),
                                          false, keys);
        Q_ASSERT(!keylistresult.error());
        Q_ASSERT(keys.size() == 1);
        delete listjob;

        auto job = openpgp()->encryptJob(/*ASCII Armor */true, /* Textmode */ true);
        Q_ASSERT(job);
        QByteArray cipherText;
        auto result = job->exec(keys, QStringLiteral("Hello World").toUtf8(), Context::AlwaysTrust, cipherText);
        delete job;
        Q_ASSERT(!result.error());
        const auto cipherString = QString::fromUtf8(cipherText);
        Q_ASSERT(cipherString.startsWith("-----BEGIN PGP MESSAGE-----"));

        /* Now decrypt */
        auto ctx = Context::createForProtocol(OpenPGP);
        ctx->setPassphraseProvider(new TestPassphraseProvider);
        ctx->setPinentryMode(Context::PinentryLoopback);
        auto decJob = new QGpgMEDecryptJob(ctx);
        QByteArray plainText;
        auto decResult = decJob->exec(cipherText, plainText);
        Q_ASSERT(!result.error());
        Q_ASSERT(QString::fromUtf8(plainText) == QStringLiteral("Hello World"));
        delete decJob;
    }

    void testSymmetricEncryptDecrypt()
    {
        auto ctx = Context::createForProtocol(OpenPGP);
        ctx->setPassphraseProvider(new TestPassphraseProvider);
        ctx->setPinentryMode(Context::PinentryLoopback);
        ctx->setArmor(true);
        ctx->setTextMode(true);
        auto job = new QGpgMEEncryptJob(ctx);
        QByteArray cipherText;
        auto result = job->exec(std::vector<Key>(), QStringLiteral("Hello symmetric World").toUtf8(), Context::AlwaysTrust, cipherText);
        delete job;
        Q_ASSERT(!result.error());
        const auto cipherString = QString::fromUtf8(cipherText);
        Q_ASSERT(cipherString.startsWith("-----BEGIN PGP MESSAGE-----"));

        killAgent(mDir.path());

        auto ctx2 = Context::createForProtocol(OpenPGP);
        ctx2->setPassphraseProvider(new TestPassphraseProvider);
        ctx2->setPinentryMode(Context::PinentryLoopback);
        auto decJob = new QGpgMEDecryptJob(ctx2);
        QByteArray plainText;
        auto decResult = decJob->exec(cipherText, plainText);
        Q_ASSERT(!result.error());
        Q_ASSERT(QString::fromUtf8(plainText) == QStringLiteral("Hello symmetric World"));
        delete decJob;
    }

private:
    /* Loopback and passphrase provider don't work for mixed encryption.
     * So this test is disabled until gnupg(?) is fixed for this. */
    void testMixedEncryptDecrypt()
    {
        auto listjob = openpgp()->keyListJob(false, false, false);
        std::vector<Key> keys;
        auto keylistresult = listjob->exec(QStringList() << QStringLiteral("alfa@example.net"),
                                          false, keys);
        Q_ASSERT(!keylistresult.error());
        Q_ASSERT(keys.size() == 1);
        delete listjob;

        auto ctx = Context::createForProtocol(OpenPGP);
        ctx->setPassphraseProvider(new TestPassphraseProvider);
        ctx->setPinentryMode(Context::PinentryLoopback);
        ctx->setArmor(true);
        ctx->setTextMode(true);
        auto job = new QGpgMEEncryptJob(ctx);
        QByteArray cipherText;
        printf("Before exec, flags: %x\n", Context::Symmetric | Context::AlwaysTrust);
        auto result = job->exec(keys, QStringLiteral("Hello symmetric World").toUtf8(),
                                static_cast<Context::EncryptionFlags>(Context::Symmetric | Context::AlwaysTrust),
                                cipherText);
        printf("After exec\n");
        delete job;
        Q_ASSERT(!result.error());
        printf("Cipher:\n%s\n", cipherText.constData());
        const auto cipherString = QString::fromUtf8(cipherText);
        Q_ASSERT(cipherString.startsWith("-----BEGIN PGP MESSAGE-----"));

        killAgent(mDir.path());

        /* Now create a new homedir which with we test symetric decrypt. */
        QTemporaryDir tmp;
        qputenv("GNUPGHOME", tmp.path().toUtf8());
        QFile agentConf(tmp.path() + QStringLiteral("/gpg-agent.conf"));
        Q_ASSERT(agentConf.open(QIODevice::WriteOnly));
        agentConf.write("allow-loopback-pinentry");
        agentConf.close();

        auto ctx2 = Context::createForProtocol(OpenPGP);
        ctx2->setPassphraseProvider(new TestPassphraseProvider);
        ctx2->setPinentryMode(Context::PinentryLoopback);
        ctx2->setTextMode(true);
        auto decJob = new QGpgMEDecryptJob(ctx2);
        QByteArray plainText;
        auto decResult = decJob->exec(cipherText, plainText);
        Q_ASSERT(!decResult.error());
        qDebug() << "Plain: " << plainText;
        Q_ASSERT(QString::fromUtf8(plainText) == QStringLiteral("Hello symmetric World"));
        delete decJob;

        killAgent(tmp.path());
        qputenv("GNUPGHOME", mDir.path().toUtf8());
    }

public Q_SLOT:

    void initTestCase()
    {
        QGpgMETest::initTestCase();
        const QString gpgHome = qgetenv("GNUPGHOME");
        qputenv("GNUPGHOME", mDir.path().toUtf8());
        Q_ASSERT(mDir.isValid());
        QFile agentConf(mDir.path() + QStringLiteral("/gpg-agent.conf"));
        Q_ASSERT(agentConf.open(QIODevice::WriteOnly));
        agentConf.write("allow-loopback-pinentry");
        agentConf.close();
        Q_ASSERT(QFile::copy(gpgHome + QStringLiteral("/pubring.gpg"),
                 mDir.path() + QStringLiteral("/pubring.gpg")));
        Q_ASSERT(QFile::copy(gpgHome + QStringLiteral("/secring.gpg"),
                 mDir.path() + QStringLiteral("/secring.gpg")));

    }

private:
    QTemporaryDir mDir;
};

QTEST_MAIN(EncryptionTest)

#include "t-encrypt.moc"