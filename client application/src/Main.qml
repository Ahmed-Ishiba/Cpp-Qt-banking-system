import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
    title: "Banking System"

    // Session state — kept here since several requests need to know who's logged in
    property string currentUsername: ""
    property bool isAdminSession: false

    Component.onCompleted: {
        networkClient.connectToServer("127.0.0.1", 12345)
    }

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: loginpage
    }

    // ============ LOGIN ============
    Component {
        id: loginpage
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10

                Label { text: "Username" }
                TextField {
                    id: usernamefield
                    placeholderText: "Enter Username"
                }
                Label { text: "Password" }
                TextField {
                    id: passwordfield
                    placeholderText: "Enter Password"
                    echoMode: TextInput.Password
                }

                RowLayout {
                    RadioButton {
                        id: user
                        checked: true
                        text: "User"
                    }
                    RadioButton {
                        id: admin
                        text: "Admin"
                    }
                }

                Label {
                    id: loginError
                    color: "red"
                    text: ""
                }

                RowLayout {
                    Button {
                        text: "Login"
                        onClicked: {
                            loginError.text = ""
                            networkClient.login(usernamefield.text, passwordfield.text)
                        }
                    }
                    Button {
                        text: "Sign Up"
                        onClicked: { stack.push(signup) }
                    }
                }
            }

            Connections {
                target: networkClient
                function onLoginResult(success, isAdmin) {
                    if (success) {
                        window.currentUsername = usernamefield.text
                        window.isAdminSession = isAdmin
                        if (isAdmin) stack.push(adminDashboard)
                        else stack.push(dashboard)
                    } else {
                        loginError.text = "Invalid username or password"
                    }
                }
                function onConnectionFailed(error) {
                    loginError.text = "Connection failed: " + error
                }
            }
        }
    }

    // ============ SIGN UP ============
    // Note: sign-up maps to "Create New User", which per the spec is Admin-only.
    // Left as local-only for now since a not-yet-logged-in user has no admin session
    // to authorize this request. Revisit once you decide how self-signup should work.
    Component {
        id: signup
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Username" }
                TextField {
                    id: usernamefield
                    placeholderText: "Enter Username"
                }
                Label { text: "Password" }
                TextField {
                    id: passwordfield
                    placeholderText: "Enter Password"
                    echoMode: TextInput.Password
                }
                Label { text: "Re-enter password" }
                TextField {
                    id: confirmfield
                    placeholderText: "Re-enter Password"
                    echoMode: TextInput.Password
                }
                Label {
                    id: signupMsg
                    color: "red"
                    text: ""
                }
                Button {
                    text: "Sign Up"
                    onClicked: {
                        if (passwordfield.text !== confirmfield.text) {
                            signupMsg.text = "Passwords do not match"
                        } else {
                            signupMsg.text = "Sign-up requires admin approval — contact an admin."
                        }
                    }
                }
                Button {
                    text: "Back to Login"
                    onClicked: { stack.pop() }
                }
            }
        }
    }

    // ============ USER DASHBOARD ============
    Component {
        id: dashboard
        Item {
            GridLayout {
                rowSpacing: 60
                columnSpacing: 100
                columns: 2
                anchors.centerIn: parent
                BankButton {
                    text: "Get account number"
                    onClicked: { stack.push(getaccountnumber) }
                }
                BankButton {
                    text: "view account balance"
                    onClicked: { stack.push(viewbalance) }
                }
                BankButton {
                    text: "view Transaction history"
                    onClicked: { stack.push(viewhistory) }
                }
                BankButton {
                    text: "make transaction"
                    onClicked: { stack.push(maketransaction) }
                }
                BankButton {
                    text: "transfer money"
                    onClicked: { stack.push(transfer) }
                }
                BankButton {
                    text: "Sign Out"
                    onClicked: { stack.push(loginpage) }
                }
            }
        }
    }

    // ---- Get Account Number (User: self only) ----
    Component {
        id: getaccountnumber
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Your account number:" }
                Label {
                    id: accNumLabel
                    text: "—"
                    font.bold: true
                }
                Button {
                    text: "Fetch"
                    onClicked: { networkClient.getAccountNumber(window.currentUsername) }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onAccountNumberResult(accountNumber) {
                    accNumLabel.text = accountNumber.length > 0 ? accountNumber : "(not found)"
                }
            }
        }
    }

    // ---- View Balance (any account number) ----
    Component {
        id: viewbalance
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Account number" }
                TextField {
                    id: accField
                    placeholderText: "e.g. 1001"
                }
                Label {
                    id: balanceLabel
                    text: ""
                }
                Button {
                    text: "Check Balance"
                    onClicked: { networkClient.viewBalance(accField.text) }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onBalanceResult(success, balance) {
                    balanceLabel.text = success ? ("Balance: " + balance) : "Account not found"
                }
            }
        }
    }

    // ---- View Transaction History ----
    Component {
        id: viewhistory
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Account number" }
                TextField {
                    id: accField
                    placeholderText: "e.g. 1001"
                }
                Label { text: "Count" }
                TextField {
                    id: countField
                    text: "5"
                    validator: IntValidator { bottom: 1 }
                }
                Button {
                    text: "Fetch History"
                    onClicked: {
                        networkClient.viewTransactionHistory(accField.text, parseInt(countField.text))
                    }
                }
                ListView {
                    id: historyList
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 150
                    model: ListModel { id: historyModel }
                    delegate: RowLayout {
                        width: historyList.width
                        Label { text: date }
                        Label { text: amount > 0 ? ("+" + amount) : amount }
                    }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onHistoryResult(historyJson) {
                    historyModel.clear()
                    var arr = JSON.parse(historyJson)
                    for (var i = 0; i < arr.length; i++) {
                        historyModel.append({ "date": arr[i].date, "amount": arr[i].Amount })
                    }
                }
            }
        }
    }

    // ---- Make Transaction (deposit/withdraw on own account) ----
    Component {
        id: maketransaction
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Amount (negative to withdraw, positive to deposit)" }
                TextField {
                    id: amountField
                    validator: IntValidator { bottom: -1000000; top: 1000000 }
                }
                Label {
                    id: resultLabel
                    text: ""
                }
                Button {
                    text: "Submit"
                    onClicked: {
                        networkClient.makeTransaction(parseInt(amountField.text))
                    }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onTransactionResult(success) {
                    resultLabel.text = success ? "Transaction successful" : "Transaction failed (insufficient funds?)"
                }
            }
        }
    }

    // ---- Transfer Money ----
    Component {
        id: transfer
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "To account number" }
                TextField {
                    id: toAccField
                    placeholderText: "e.g. 1002"
                }
                Label { text: "Amount" }
                TextField {
                    id: amountField
                    validator: IntValidator { bottom: 1 }
                }
                Label {
                    id: resultLabel
                    text: ""
                }
                Button {
                    text: "Transfer"
                    onClicked: {
                        networkClient.transferAmount(toAccField.text, parseInt(amountField.text))
                    }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onTransferResult(success) {
                    resultLabel.text = success ? "Transfer successful" : "Transfer failed (insufficient funds or bad account?)"
                }
            }
        }
    }

    // ============ ADMIN DASHBOARD ============
    Component {
        id: adminDashboard
        Item {
            GridLayout {
                rowSpacing: 60
                columnSpacing: 100
                columns: 2
                anchors.centerIn: parent
                BankButton {
                    text: "Get account number"
                    onClicked: { stack.push(getaccountnumberAdmin) }
                }
                BankButton {
                    text: "view account balance"
                    onClicked: { stack.push(viewbalance) }
                }
                BankButton {
                    text: "view Transaction history"
                    onClicked: { stack.push(viewhistory) }
                }
                BankButton {
                    text: "database"
                    onClicked: { stack.push(viewdatabase) }
                }
                BankButton {
                    text: "create new user"
                    onClicked: { stack.push(createUser) }
                }
                BankButton {
                    text: "Update User"
                    onClicked: { stack.push(updateUser) }
                }
                BankButton {
                    text: "Delete User"
                    onClicked: { stack.push(deleteUserPage) }
                }
                BankButton {
                    text: "Sign Out"
                    onClicked: { stack.push(loginpage) }
                }
            }
        }
    }

    // ---- Get Account Number (Admin: lookup any username) ----
    Component {
        id: getaccountnumberAdmin
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Username to look up" }
                TextField {
                    id: targetUserField
                    placeholderText: "e.g. johndoe"
                }
                Label {
                    id: accNumLabel
                    text: "—"
                    font.bold: true
                }
                Button {
                    text: "Fetch"
                    onClicked: { networkClient.getAccountNumberAdmin(targetUserField.text) }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onAccountNumberResult(accountNumber) {
                    accNumLabel.text = accountNumber.length > 0 ? accountNumber : "(not found)"
                }
            }
        }
    }

    // ---- View Whole Bank Database ----
    Component {
        id: viewdatabase
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Button {
                    text: "Load Database"
                    onClicked: { networkClient.viewBankDatabase() }
                }
                ListView {
                    id: dbList
                    Layout.preferredWidth: 400
                    Layout.preferredHeight: 250
                    model: ListModel { id: dbModel }
                    delegate: RowLayout {
                        width: dbList.width
                        Label { text: uname }
                        Label { text: accnum }
                        Label { text: bal }
                    }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onBankDatabaseResult(data) {
                    dbModel.clear()
                    var arr = JSON.parse(data)
                    for (var i = 0; i < arr.length; i++) {
                        dbModel.append({
                            "uname": arr[i].username,
                            "accnum": arr[i].accountNumber,
                            "bal": arr[i].balance
                        })
                    }
                }
            }
        }
    }

    // ---- Create New User ----
    Component {
        id: createUser
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Username" }
                TextField { id: newUsernameField }
                Label { text: "Password" }
                TextField { id: newPasswordField; echoMode: TextInput.Password }
                Label { text: "Account Number" }
                TextField { id: newAccNumField }
                Label {
                    id: resultLabel
                    text: ""
                }
                Button {
                    text: "Create"
                    onClicked: {
                        var payload = {
                            "username": newUsernameField.text,
                            "password": newPasswordField.text,
                            "accountNumber": newAccNumField.text
                        }
                        networkClient.createNewUser(JSON.stringify(payload))
                    }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onCreateUserResult(success) {
                    resultLabel.text = success ? "User created" : "Failed — username or account number already exists"
                }
            }
        }
    }

    // ---- Update User ----
    Component {
        id: updateUser
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Account number to update" }
                TextField { id: targetAccField }
                Label { text: "New username (leave blank to skip)" }
                TextField { id: newUnameField }
                Label { text: "New password (leave blank to skip)" }
                TextField { id: newPassField; echoMode: TextInput.Password }
                Label { text: "New balance (leave blank to skip)" }
                TextField { id: newBalField; validator: IntValidator {} }
                Label {
                    id: resultLabel
                    text: ""
                }
                Button {
                    text: "Update"
                    onClicked: {
                        var payload = {}
                        if (newUnameField.text.length > 0) payload["username"] = newUnameField.text
                        if (newPassField.text.length > 0) payload["password"] = newPassField.text
                        if (newBalField.text.length > 0) payload["balance"] = parseInt(newBalField.text)
                        networkClient.updateUser(targetAccField.text, JSON.stringify(payload))
                    }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onUpdateUserResult(success) {
                    resultLabel.text = success ? "User updated" : "Update failed — account not found?"
                }
            }
        }
    }

    // ---- Delete User ----
    Component {
        id: deleteUserPage
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                Label { text: "Account number to delete" }
                TextField { id: delAccField }
                Label {
                    id: resultLabel
                    text: ""
                }
                Button {
                    text: "Delete"
                    onClicked: { networkClient.deleteUser(delAccField.text) }
                }
                Button {
                    text: "Back"
                    onClicked: { stack.pop() }
                }
            }
            Connections {
                target: networkClient
                function onDeleteUserResult(success) {
                    resultLabel.text = success ? "User deleted" : "Delete failed — account not found?"
                }
            }
        }
    }
}