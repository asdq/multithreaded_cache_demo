-- Create table `records` in database test
CREATE TABLE IF NOT EXISTS test.`records` (
    `key` varchar(20) NOT NULL,
    `data` text,
    PRIMARY KEY (`key`)
);
