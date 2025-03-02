document.addEventListener("DOMContentLoaded", () => {
    const choices = document.querySelectorAll('.choice');
    const resultText = document.getElementById("result");

    // 玩家选择
    choices.forEach(button => {
        button.addEventListener('click', () => {
            const playerChoice = button.getAttribute("data-choice");
            playGame(playerChoice);
        });
    });

    // 电脑随机选择
    function getComputerChoice(){
        const options = ["rock", "paper", "scissors"];
        const randomIndex = Math.floor(Math.random() * options.length);
        console.log("电脑选择索引:", randomIndex, "选择:", options[randomIndex]); // 调试用
        return options[randomIndex];
    }

    // 计算胜负
    function determineWinner(player, computer){
        if(player === computer){
            return "平局！😐";
        } else if (
            (player === "rock" && computer === "scissors") ||
            (player === "paper" && computer === "rock") ||
            (player === "scissors" && computer === "paper")
        ){
            return "你赢了！🎉";
        } else {
            return "你输了！😢";
        }
    }

    // 运行游戏
    function playGame(playerChoice){
        const computerChoice = getComputerChoice();
        const result = determineWinner(playerChoice, computerChoice);

        resultText.innerHTML = `你出 ${getEmoji(playerChoice)}, 电脑出 ${getEmoji(computerChoice)} <br> ${result}`;
    }

    // 让结果更有趣（加入表情）
    function getEmoji(choice){
        const emojis = {
            rock: "✊ 石头",
            paper: "✋ 布",
            scissors: "✌ 剪刀"
        };
        return emojis[choice] || "";
    }
});
