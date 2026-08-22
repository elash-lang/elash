from dataclasses import dataclass, field
from typing import Literal, Optional
from pathlib import Path

TestType  = Literal['positive', 'negative']
TestStage = Literal['compilation', 'linking', 'runtime']
Severity  = Literal['error', 'warning', 'note']

@dataclass
class Timeouts:
    compile: float
    link:    float
    runtime: float

@dataclass
class DiagExpectation:
    severity: Severity
    code:     Optional[str]
    lines:    Optional[list[int]]
    message:  Optional[str] = None

@dataclass(order=True)
class TestCase:
    name: str
    path: Path = field(compare=False)
    type: TestType

@dataclass
class PositiveTestExpectation:
    exitcode:    int
    stdout:      str
    stderr:      str

    diags: list[DiagExpectation]

@dataclass
class NegativeTestExpectation:
    diags: list[DiagExpectation]
    ignore_unexpected: bool = False

TestExpectation = PositiveTestExpectation | NegativeTestExpectation


@dataclass
class FinishedResult:
    exitcode:    int
    stdout:      str
    stderr:      str
    stage:       TestStage
    compilation_result: Optional['FinishedResult'] = None

@dataclass
class TimedOutResult:
    stage: TestStage

TestResult = FinishedResult | TimedOutResult

