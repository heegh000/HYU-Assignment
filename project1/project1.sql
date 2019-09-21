#1. Grass 타입의 포켓몬의 이름을 사전순으로 출력하세요
SELECT name
FROM Pokemon
WHERE type = 'Grass'
ORDER BY type;

#2. Brown City나 Rainbow City 출신 트레이너의 이름을 사전순으로 출력하세요
SELECT name
FROM Trainer
WHERE hometown = 'Brown City'
UNION
SELECT name
FROM Trainer
WHERE hometown = 'Rainbow City'
ORDER BY name;

#3. 모든 포켓몬의 type을 중복없이 사전순으로 출력하세요
SELECT DISTINCT type
FROM Pokemon
ORDER BY type;

#4. 도시의 이름이 B로 시작하는 모든 도시의 이름을 사전순으로 출력하세요
SELECT name
FROM City
WHERE name LIKE 'B%'
ORDER BY name;

#5. 이름이 M으로 시작하지 않는 트레이너의 고향을 사전순으로 출력하세요
SELECT hometown
FROM Trainer
WHERE NOT name LIKE 'M%'
ORDER BY hometown;

#6. 잡힌 포켓몬 중 가장 레벨이 높은 포켓몬의 별명을 사전순으로 출력하세요
SELECT nickname
FROM CatchedPokemon
WHERE level >= ALL(
                  SELECT level
                  FROM CatchedPokemon)
ORDER BY nickname;

#7. 포켓몬의 이름이 알파벳 모음으로 시작하는 포켓몬의 이름을 사전순으로 출력하세요
SELECT name
FROM Pokemon
WHERE name LIKE 'A%'
	OR name LIKE 'E%'
	OR name LIKE 'I%'
	OR name LIKE 'O%'
	OR name LIKE 'U%'
ORDER BY name;

#8. 잡힌 포켓몬의 평균 레벨을 출력하세요
SELECT AVG(level)
FROM CatchedPokemon;

#9. Yellow가 잡은 포켓몬의 최대 레벨을 출력하세요
SELECT MAX(level)
FROM CatchedPokemon, Trainer
WHERE owner_id = Trainer.id 
	AND name = 'Yellow';


#10 .트레이너의 고향 이름을 중복없이 사전순으로 출력하세요
SELECT DISTINCT hometown
FROM Trainer
ORDER BY hometown;


#11. 닉네임이 A로 시작하는 포켓몬을 잡은 트레이너의 이름과 포켓몬의 닉네임을 트레이너의 이름의 사전순으로 출력하세요
SELECT Trainer.name, nickname
FROM Trainer, CatchedPokemon 
WHERE nickname LIKE 'A%'
	AND Trainer.id = owner_id
ORDER BY name;

#12. Amazon 특성을 가진 도시의 리더의 트레이너 이름을 출력하세요
SELECT Trainer.name
FROM Trainer, Gym, City
WHERE id = leader_id 
	AND City.name = Trainer.hometown
	AND description = 'Amazon';

#13. 불속성 포켓몬을 가장 많이 잡은 트레이너의 id와, 그 트레이너가 잡은 불속성 포켓몬의 수를 출력하세요                       
SELECT fire.fire_num_name, fire.fire_num 
FROM (SELECT Trainer.name AS fire_num_name, COUNT(*) AS fire_num
	FROM CatchedPokemon, Pokemon, Trainer
	WHERE pid = Pokemon.id AND type = 'Fire' AND owner_id = Trainer.id
	GROUP BY owner_id) AS fire
WHERE fire.fire_num >= ALL(SELECT COUNT(*)
                           FROM CatchedPokemon, Pokemon, Trainer
                           WHERE pid = Pokemon.id AND type = 'Fire' AND owner_id = Trainer.id
                           GROUP BY owner_id);


#14. 포켓몬 ID가 한 자리 수인 포켓몬의 type을 중복 없이 포켓몬 ID의 내림차순으로 출력하세요
SELECT type
FROM Pokemon
WHERE id < 10
ORDER BY id DESC;

#15. 포켓몬의 type이 Fire이 아닌 포켓몬의 수를 출력하세요
SELECT COUNT(id)
FROM Pokemon
WHERE NOT type = 'Fire';


#16. 진화하면 id가 작아지는 포켓몬의 진화 전 이름을 사전순으로 출력하세요
SELECT name
FROM Pokemon, Evolution
WHERE before_id = id AND before_id > after_id;

#17. 트레이너에게 잡힌 모든 물속성 포켓몬의 평균 레벨을 출력하세요
SELECT AVG(level)
FROM CatchedPokemon, Pokemon
WHERE pid = Pokemon.id AND type = 'Water';

#18. 체육관 리더가 잡은 모든 포켓몬 중 레벨이 가장 높은 포켓몬의 별명을 출력하세요
SELECT nickname
FROM CatchedPokemon, 
     (SELECT name, id
      FROM Trainer, Gym
      WHERE id = leader_id) AS gym_leader
WHERE gym_leader.id = CatchedPokemon.owner_id 
      AND CatchedPokemon.level >= ALL(SELECT level
                                      FROM Trainer, Gym, CatchedPokemon
                                      WHERE Trainer.id = leader_id
                                         AND Trainer.id = owner_id);

#19. Blue city 출신 트레이너들 중 잡은 포켓몬들의 레벨의 평균이 가장 높은 트레이너의 이름을 사전순으로 출력하세요
SELECT trainer_name
FROM ( SELECT AVG(level) AS avg_level, Trainer.name AS trainer_name
       FROM Trainer, City, CatchedPokemon
       WHERE hometown = City.name 
           AND Trainer.id = owner_id
           AND City.name = 'Blue City'
       GROUP by owner_id) AS avg_level_table
WHERE avg_level >= ALL (
                       SELECT AVG(level)
                       FROM Trainer, City, CatchedPokemon
                       WHERE hometown = City.name 
                           AND Trainer.id = owner_id
                           AND City.name = 'Blue City'
                       GROUP by owner_id);



#20. 같은 출신이 없는 트레이너들이 잡은 포켓몬중 진화가 가능하고 Electric 속성을 가진 포켓몬의 이름을 출력하세요
SELECT Pokemon.name
FROM CatchedPokemon, Pokemon,
     (SELECT DISTINCT Pokemon.id as elect_evol_id
      FROM Pokemon, Evolution
      WHERE type = 'Electric'
            AND (Pokemon.id = before_id OR Pokemon.id = after_id)
            AND (before_id, after_id) IN (SELECT * FROM Evolution)) AS elect_evol,
      (SELECT id as no_same_hometown_id
       FROM Trainer, City,
            (SELECT COUNT(*) as home_num, hometown
             FROM Trainer 
             GROUP BY hometown) as home_num_table
       WHERE Trainer.hometown = City.name
             AND home_num < 2
             AND home_num_table.hometown = Trainer.hometown) AS no_same_hometown
WHERE Pokemon.id = elect_evol_id 
      AND pid = Pokemon.id
      AND no_same_hometown_id = owner_id;



#21. 관장들의 이름과 각 관장들이 잡은 포켓몬들의 레벨 합을 레벨 합의 내림차 순으로 출력하세요
SELECT gym_leader, level_sum
FROM (SELECT Trainer.name AS gym_leader, SUM(level) AS level_sum
      FROM Trainer, Gym, CatchedPokemon
      WHERE Trainer.id = Gym.leader_id
             AND Trainer.id = owner_id
      GROUP BY Trainer.name) AS gym_leader_table
ORDER BY gym_leader_table.level_sum DESC;

#22. 가장 트레이너가 많은 고향의 이름을 출력하세요.
SELECT home_name
FROM (SELECT COUNT(*) AS home_num, Trainer.hometown as home_name
      FROM Trainer
      GROUP BY hometown) as home_num_table
WHERE home_num >= ALL(SELECT COUNT(*)
                      FROM Trainer
                      GROUP BY hometown)

#23. Sangnok City 출신 트레이너와 Brown City 출신 트레이너가 공통으로 잡은 포켓몬의 이름을 중복을 제거하여 사전순으로 출력하세요
SELECT DISTINCT Pokemon.name
FROM Pokemon,
     (SELECT pid as sangnok_pid
      FROM CatchedPokemon, Trainer
      WHERE Trainer.id = owner_id
            AND Trainer.hometown = 'Sangnok City') as sangnok_catched_poke,
      (SELECT pid as brown_pid
       FROM CatchedPokemon, Trainer
       WHERE Trainer.id = owner_id
            AND Trainer.hometown = 'Brown City') as brown_catched_poke
WHERE Pokemon.id = brown_pid
      AND Pokemon.id = sangnok_pid
ORDER BY Pokemon.name;


#24. 이름이 P로 시작하는 포켓몬을 잡은 트레이너 중 상록 시티 출신인 트레이너의 이름을 사전순으로 모두 출력하세요
SELECT Trainer.name
FROM City,Trainer,
     (SELECT Trainer.id as p_poke_trainer_id
      FROM Trainer, CatchedPokemon, Pokemon
      WHERE Trainer.id = owner_id
            AND pid = Pokemon.id
            AND Pokemon.name LIKE 'P%') as p_poke_trainer
WHERE Trainer.id = p_poke_trainer_id
    AND City.name = Trainer.hometown
    AND City.name = 'Sangnok City'
ORDER BY Trainer.name;

#25. 트레이너의 이름과 그 트레이너가 잡은 포켓몬의 이름을 출력하세요. 트레이너 이름의 사전 순으로 정렬하고, 각 트레이너가 잡은 포켓몬 간에도 사전 순으로 정렬하세요.
SELECT Trainer.name, Pokemon.name
FROM Trainer, CatchedPokemon, Pokemon
WHERE Trainer.id = owner_id
      AND pid = Pokemon.id
ORDER BY Trainer.name, Pokemon.name;

#26. 2단계 진화만 가능한 포켓몬의 이름을 사전순으로 출력하세요
SELECT Pokemon.name
FROM Pokemon ,Evolution AS evo
WHERE Pokemon.id = evo.before_id
      AND NOT(EXISTS (SELECT after_id
                      FROM Evolution
                      WHERE evo.before_id = after_id)
             OR
             EXISTS (SELECT before_id
                      FROM Evolution
                      WHERE evo.after_id = before_id))
ORDER BY Pokemon.name;

#27. 상록 시티의 관장이 잡은 포켓몬들 중 포켓몬의 타입이 WATER 인 포켓몬의 별명을 사전순으로 출력하세요
SELECT nickname
FROM CatchedPokemon,
    (SELECT pid
     FROM CatchedPokemon, Trainer, Gym
     WHERE Trainer.id = Gym.leader_id
           AND Trainer.id = owner_id
           AND Gym.City = 'Sangnok City') AS sangnok_gym_catch_poke
WHERE CatchedPokemon.pid = sangnok_gym_catch_poke.pid
      AND CatchedPokemon.pid IN (SELECT id
                                FROM Pokemon
                                WHERE type = 'Water')
ORDER BY nickname;

#28. 트레이너들이 잡은 포켓몬 중 진화한 포켓몬이 3마리 이상인 경우 해당 트레이너의 이름을 사전순으로 출력하세요
SELECT name
FROM Trainer,
     (SELECT COUNT(CatchedPokemon.id) as after_evo_num, Trainer.id AS after_evo_table_id
      FROM Trainer, CatchedPokemon, Evolution
      WHERE owner_id = Trainer.id
             AND pid = after_id
      GROUP BY Trainer.id) AS after_evo_table
WHERE Trainer.id = after_evo_table_id
      AND after_evo_num >= 3
ORDER BY name;

#29. 어느 트레이너에게도 잡히지 않은 포켓몬의 이름을 사전 순으로 출력하세요
SELECT name
FROM Pokemon
WHERE Pokemon.id <> ALL (SELECT pid
           FROM Trainer, CatchedPokemon
           WHERE owner_id = Trainer.id)
ORDER BY name;

#30. 각 출신 도시별로 트레이너가 잡은 포켓몬중 가장 레벨이 높은 포켓몬의 레벨을 내림차 순으로 출력하세요.
SELECT Max(level)
FROM CatchedPokemon, Trainer, Pokemon
WHERE owner_id = Trainer.id
	AND Pokemon.id = pid
GROUP BY Trainer.hometown
ORDER BY Max(level) DESC;



#31. 포켓몬 중 3단 진화가 가능한 포켓몬의 ID 와 해당 포켓몬의이름을 1단진화 형태 포켓몬의이름, 2단 진화 형태 포켓몬의 이름, 3단 진화 형태 포켓몬의 이름을 ID 의 오름차순으로 출력하세요
SELECT evo_id1,
       (SELECT name FROM Pokemon WHERE id = evo_id1),
       (SELECT name FROM Pokemon WHERE id = evo_id2),
       (SELECT name FROM Pokemon WHERE id = evo_id3)
FROM (SELECT evo1.before_id AS evo_id1, evo2.before_id AS evo_id2, evo2.after_id AS evo_id3
      FROM Evolution evo1, Evolution evo2
      WHERE evo1.after_id = evo2.before_id
      ORDER BY evo1.before_id) AS three_evo
ORDER BY evo_id1;
